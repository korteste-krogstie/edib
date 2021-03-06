#pragma once
#include<cmath>
#include<cstdint>
#include<limits>
#include<map>
#include<memory>
#include<string>
#include<vector>

#include<time.h>

namespace utc{
/** \brief time
 * basic types for time handling
 * we use linear time, i.e. time is just a number
 * on the timeaxis, utc. Timeaxis zero is at 1970-01-01 00:00:00 (unix time).
 * resolution is 1 second, integer.
 * also define min max and no_time
 *
 * The advantage with the definition is that it is well defined and commonly known in all platforms
 * Also considered: moving to std::chrono, would enable a strict time arithmetic regime, of little advantage in this
 * context, and at the cost of compile time (also consider the python/api part).
 */
using time_t=int64_t;       /// time_t is typedef'd as a __time64_t, which is an __int64., happens to be equal to EnkiTime
using timespan_t=int64_t;   /// timespan is typdedef'd as a time (thus __int64)

/** \brief deltahours
 * \param n number of hours
 * \return timespan representing number of hours specified
 */
inline timespan_t deltahours(int n) { return n*3600; }

/** \brief deltaminutes
 * \param n number of minutes
 * \return timespan_t representing number of minutes specified
 */
inline timespan_t deltaminutes(int n) { return n*60; }

/** \brief max_time represent the maximum time
 */
static constexpr time_t max_time = std::numeric_limits<time_t>::max(); 

/** \brief min_time represent the minimum time (equal to -max_time)
 */
static constexpr time_t min_time = std::numeric_limits<time_t>::lowest()+1;  /// min 64bit int

/** \brief no_time represents 'NaN' time, null time, not valid time
 */
static constexpr time_t no_time = std::numeric_limits<time_t>::lowest(); //TODO: select number to use...
//static_assert(min_time-1 == max_time,"");

/** \brief timeNow, time_now()
 *  \return current systemclock time
 */
inline time_t time_now() {return (time_t)time(0); }

inline bool is_valid(time_t t) {return t != no_time;}

/** \brief period_t is defined
 *  as period_t on the time space, like
 * [start..end>, where end >=start to be valid
 *
 */
struct period_t{
	time_t start;
	time_t end;
	period_t(time_t start, time_t end): start(start), end(end) {}
	period_t():start(no_time),end(no_time) {}
	timespan_t timespan() const {	return end - start; }
	bool valid() const { return start != no_time && end != no_time && start <= end; }
	bool operator==(const period_t &p) const { return start == p.start &&  end == p.end; }
	bool operator!=(const period_t &p) const {return ! (*this==p);}
	bool contains(time_t t) const {return is_valid(t)&&valid()?t>=start && t<end:false;}
	bool overlaps(const period_t& p) const {return !( (p.start >= end) || (p.end <= start) )?true:false; }
	std::string to_string() const;
};
inline bool is_valid(const period_t &p) {return p.valid();}

namespace zone{
	/**\brief zone handling, basically just a wrapper that hides
	* the fact we are using boost::date_time for getting/providing tz info
	*/
	template<typename tz>
	struct tz_info {
		typedef tz tz_type_t;
		std::string name() const {return "UTC";}
		timespan_t base_offset() const {return timespan_t(0);}
		timespan_t utc_offset(time_t t) const {return timespan_t(0);}
		bool is_dst(time_t t) const {return false;}
	};


	/**\brief The tz_table is a table driven approach where each year in a specified range have
	 * a dst information containing start/end, and the applied dst offset.
	 * This approach allows to have historical correct dst-rules, at minor space/time overhead
	 * E.g. Norway, summertime rules are changed to EU defs. in 1996
	 * first time applied was 1916, then 1943-1945, then 1959-1965, then 1980-to 1996,
	 * where eu rules was introduced.
	 */
	class tz_table {
		int start_year;
		std::string tz_name;
		std::vector<period_t> dst;
		std::vector<timespan_t> dt;

	  public:
		/**\brief construct a tz_table using a information from provided Tz adapter
		 *\tparam Tz a type that provids dst_start(year),dst_end(year), dst_offset(year)
		 *\param tz const ref to provide tz information that will be used to construct a table driven interpretation
		 *\param start_year default 1905 (limit of posix time is 1901) for which the dst table is constructed
		 *\param n_years default 200 giving range from 1905 to 2105 with dst info.
		 */
		template<typename Tz>
		tz_table(const Tz& tz ,int start_year=1905,size_t n_years=200):start_year(start_year) {
			for(int y=start_year;y<int(start_year+n_years);++y) {
				dst.emplace_back(tz.dst_start(y),tz.dst_end(y));
				dt.push_back(tz.dst_offset(y));
			}
			tz_name=tz.name();
		}
		/**\brief construct a simple dst infotable with no dst, just tz-offset
		* suitable for non-dst time-zones and data-exchange.
		* \param dt of type timespan_t, positive for tz east of GMT
		*/
		tz_table(timespan_t dt):start_year(0) {
			char s[100];sprintf(s,"UTC%+02d",int(dt/deltahours(1)));
			tz_name=s;
		}
		inline bool is_dst() const {return dst.size()>0;}
		std::string name() const {return tz_name;}
		time_t dst_start(int year) const {return is_dst()?dst[year-start_year].start:no_time;}
		time_t dst_end(int year) const {return is_dst()?dst[year-start_year].end:no_time;}
		timespan_t dst_offset(time_t t) const ;
	};

	/**\brief a table driven tz_info, using the tz_table implementation */
	template<>
	struct tz_info<tz_table> {
		timespan_t base_tz;
		tz_table tz;
		tz_info(timespan_t base_tz):base_tz(base_tz),tz(base_tz) {}
		tz_info(timespan_t base_tz,const tz_table&tz):base_tz(base_tz),tz(tz) {}
		std::string name() const {return tz.name();}
		timespan_t base_offset() const {return base_tz;}
		timespan_t utc_offset(time_t t) const {return base_tz + tz.dst_offset(t);}
		bool is_dst(time_t t) const {return tz.dst_offset(t)!=timespan_t(0);}
	};

	typedef tz_info<tz_table> tz_info_t;///< tz_info type most general, supports all kinds of tz, at a minor extra cost.
	typedef std::shared_ptr<tz_info_t> tz_info_t_;///< convinience, the shared ptr. version

	/** \brief time zone database class that provides std::shared_ptr of predefined tz_info_t objects */
	struct tz_info_database {

		/** \brief load from compile time iso db as per boost 1.60 */
		void load_from_iso_db();

		/** \brief load from file that contains all descriptions, ref. boost::date_time for format */
		void load_from_file(const std::string filename);

		/** \brief add one entry, using a specified region_name like Europe/Copenhagen, and a posix description std::string, ref boost::date_time for spec */
		void add_tz_info(std::string region_name,std::string posix_tz_string);

		/** \brief returns a std::shared_ptr to tz_info_t given time-zone region name like Europe/Copenhagen */
		std::shared_ptr<tz_info_t> tz_info_from_region(const std::string &region_name) const {
			auto f=region_tz_map.find(region_name);
			if( f!= region_tz_map.end()) return f->second;
			throw std::runtime_error(std::string("tz region '")+region_name + std::string("' not found"));
		}

		/** \brief returns a std::shared_ptr to tz_info_t given time-zone name like CET */
		std::shared_ptr<tz_info_t> tz_info_from_name(const std::string &name) const {
			auto f=name_tz_map.find(name);
			if( f!= name_tz_map.end()) return f->second;
			throw std::runtime_error(std::string("tz name '")+name + std::string("' not found"));
		}
		std::vector<std::string> get_region_list() const {
			std::vector<std::string> r;r.reserve(region_tz_map.size());
			for(const auto& c:region_tz_map)
				r.push_back(c.first);
			return r;
		}
		std::vector<std::string> get_name_list() const {
			std::vector<std::string> r;r.reserve(region_tz_map.size());
			for(const auto& c:name_tz_map)
				r.push_back(c.first);
			return r;
		}

		std::map<std::string,std::shared_ptr<tz_info_t>> region_tz_map;///< map from Europe/Copenhagen to tz
		std::map<std::string,std::shared_ptr<tz_info_t>> name_tz_map;///< map from CET to tz (same tz as Europe/Copenhagen)
	};

}
/** \brief YMDhms, simple structure that contains calendar coordinates.
 * Contains year,month,day,hour,minute, second,
 * for ease of constructing time.
 *\note the constructor do a range check for Y M D h m s, and throws if fail.
 *
 */
struct YMDhms {
	static const int YEAR_MAX= 9999;
	static const int YEAR_MIN=-9999;
	YMDhms():year(0), month(0), day(0), hour(0), minute(0), second(0) {}
	YMDhms(int Y, int M=1, int D=1, int h=0, int m=0, int s=0) : year(Y), month(M), day(D), hour(h), minute(m), second(s)  {
		if(!is_valid())
			throw std::runtime_error("calendar coordinates failed simple range check for one or more item");
	}

	int year; int month; int day; int hour; int minute; int second;
	///< just check that YMDhms are within reasonable ranges,\note it might still be an 'invalid' date!
	bool is_valid_coordinates() const 
	{
		return !(year<YEAR_MIN || year>YEAR_MAX || month<1 || month>12 ||day<1 || day>31 ||hour<0 || hour>23 || minute<0 ||minute>59||second<0 ||second>59);}
	///< if a 'null' or valid_coordinates
	bool is_valid() const { return is_null() || is_valid_coordinates(); }
	bool is_null() const { return year == 0 && month == 0 && day == 0 && hour == 0 && minute == 0 && second == 0; }
	bool operator==(const YMDhms& x) const {
		return x.year == year && x.month == month && x.day == day && x.hour == hour
			   && x.minute == minute && x.second == second;
	}
	static YMDhms max() {return YMDhms(YEAR_MAX,12,31,23,59,59);}
	static YMDhms min() {return YMDhms(YEAR_MIN,12,31,23,59,59);}
};
/** \brief Calendar deals with the concept of human calendar.
 *
 * Please notice that although the calendar concept is complete,
 * we only implement features as needed in the core and interfaces.
 *
 * including:
 * -# Conversion between the calendar coordinates YMDhms and time, taking  any timezone and DST into account
 * -# Calendar constants, timespan_t like values for Year,Month,Week,Day,Hour,Minute,Second
 * -# Calendar arithmetic, like adding calendar units, e.g. day,month,year etc.
 * -# Calendar arithmetic, like trim/truncate a time down to nearest timespan_t/calendar unit. eg. day
 * -# Calendar arithmetic, like calculate difference in calendar units(e.g days) between two time points
 * -# Calendar Timezone and DST handling
 * -# Converting time_t to std::string and vice-versa
 */
struct cal_t{
	// these do have calendar sematics(could/should be separate typed/enum instad)
	static constexpr timespan_t YEAR=365*24*3600L;
	static constexpr timespan_t MONTH=30*24*3600L;
	static constexpr timespan_t WEEK = 7*24*3600L;
	static constexpr timespan_t DAY =  1*24*3600L;
	// these are just timespan_t constexprants with no calendar semantics
	static constexpr timespan_t HOUR = 3600L;
	static constexpr timespan_t MINUTE = 60L;
	static constexpr timespan_t SECOND = 1L;

	static constexpr int UnixDay = 2440588;///< Calc::julian_day_number(ymd(1970,01,01));
	static constexpr time_t UnixSecond = 86400LL * (time_t)UnixDay;///<Calc::julian_day_number(ymd(1970,01,01));

	// Snapped from boost gregorian_calendar.ipp
	static inline unsigned long day_number(const YMDhms& ymd) {
		unsigned short a = static_cast<unsigned short>((14 - ymd.month) / 12);
		unsigned short y = static_cast<unsigned short>(ymd.year + 4800 - a);
		unsigned short m = static_cast<unsigned short>(ymd.month + 12 * a - 3);
		unsigned long  d = ymd.day + ((153 * m + 2) / 5) + 365 * y + (y / 4) - (y / 100) + (y / 400) - 32045;
		return d;
	}

	static inline YMDhms from_day_number(unsigned long dayNumber) {
		int a = dayNumber + 32044;
		int b = (4 * a + 3) / 146097;
		int c = a - ((146097 * b) / 4);
		int d = (4 * c + 3) / 1461;
		int e = c - (1461 * d) / 4;
		int m = (5 * e + 2) / 153;
		unsigned short day = static_cast<unsigned short>(e - ((153 * m + 2) / 5) + 1);
		unsigned short month = static_cast<unsigned short>(m + 3 - 12 * (m / 10));
		int year = static_cast<unsigned short>(100 * b + d - 4800 + (m / 10));
		return YMDhms(year, month, day);
	}
	static int day_number(time_t t) {
		return (int)(int)((UnixSecond + t) / DAY);
	}
	static inline timespan_t seconds(int h, int m, int s) { return h*HOUR + m*MINUTE + s*SECOND; }

	zone::tz_info_t_ tz_info;
	/**\brief construct a timezone with standard offset, no dst, name= UTC+01 etc. */
	cal_t(timespan_t tz=0): tz_info(new zone::tz_info_t(tz)) {}
	/**\brief construct a timezone from tz_info shared ptr provided from typically zone db */
	cal_t(zone::tz_info_t_ tz_info):tz_info(tz_info) {}

	/**\brief construct a timezone based on region id
	 * uses internal tz_info_database to lookup the name.
	 * \param region_id, like Europe/Oslo, \sa zone::tz_info_database
	 */
	cal_t(std::string region_id);
	/**\brief get list of available time zone region */
	static std::vector<std::string> region_id_list();

	/**\brief construct time from calendar coordinates YMDhms
	 *
	 * If the YMDhms is invalid, runtime_error is thrown.
	 * Currently just trivial checks is done.
	 *
	 * \param c YMDhms that has to be valid calendar coordinates.
	 * \note special values of YMDhms, like max,min,null is mapped to corresponding time concepts.
	 * \sa YMDhms
	 * \return time
	 */
	time_t time(YMDhms c) const;

	///<short hand for calendar::time(YMDhms)
	time_t time(int Y,int M=1,int D=1,int h=0,int m=0,int s=0) const {
		return time(YMDhms(Y,M,D,h,m,s));
	}
	/**\brief returns *utc_year* of t \note for internal dst calculations only */
	static inline int utc_year(time_t t) {
		if(t == no_time  ) throw std::runtime_error("year of no_time");
		if(t == max_time ) return YMDhms::YEAR_MAX;
		if(t == min_time ) return YMDhms::YEAR_MIN;
		return from_day_number(day_number(t)).year;
	}
	/**\brief return the calendar units of t taking timezone and dst into account
	 *
	 * Special time values, no_time max_time, min_time are mapped to corresponding
	 * YMDhms special values.
	 * \sa YMDhms
	 * \return calendar units YMDhms
	 */
	YMDhms calendar_units(time_t t) const ;

	///< returns  0=sunday, 1= monday ... 6=sat.
	int day_of_week(time_t t) const ;

	///< returns day_of year, 1..365..
	size_t day_of_year(time_t t) const ;

	///< returns the month of t, 1..12, -1 of not valid time
	int month(time_t t) const ;

	///< returns a readable iso standard std::string
	std::string to_string(time_t t) const;

	///< returns a readable period_t with time as for calendar::to_string
	std::string to_string(period_t p) const;

	// calendar arithmetic
	/**\brief round down (floor) to nearest time with deltaT resolution.
	 *
	 * If delta T is calendar::DAY,WEEK,MONTH,YEAR it do a time-zone semantically
	 * correct rounding.
	 * if delta T is any other number, e.g. minute/hour, the result is similar to
	 * integer truncation at the level of delta T.
	 * \param t time_t to be trimmed
	 * \param deltaT timespan_t specifying the resolution, use calendar::DAY,WEEK,MONTH,YEAR specify calendar specific resolutions
	 * \return a trimmed time
	 */
	time_t trim(time_t t, timespan_t deltaT) const ;

	/**\brief calendar semantic add
	 *
	 *  conceptually this is similar to t + deltaT*n
	 *  but with deltaT equal to calendar::DAY,WEEK,MONTH,YEAR
	 *  and/or with dst enabled time-zone the variation of length due to dst
	 *  or month/year length is taken into account
	 *  e.g. add one day, and calendar have dst, could give 23,24 or 25 hours due to dst.
	 *  similar for week or any other time steps.
	 *
	 *  \sa calendar::diff_units
	 *
	 *  \note DST -if the calendar include dst, following rules applies:
	 *   -# transition hour 1st hour after dst has changed
	 *   -# if t and resulting t have different utc-offsets, the result is adjusted dst adjustment with the difference.
	 *
	 *
	 * \param t time
	 * \param delta T timespan_t, that can be any, but with calendar::DAY,WEEK,MONTH,YEAR calendar semantics applies
	 * \param n numer of delta T to add, can be negative
	 * \return new calculated time
	 */
	time_t add(time_t t, timespan_t deltaT, long n) const ;

	/**\brief calculate the distance t1..t2 in specified units
	 *
	 * The function takes calendar semantics when deltaT is calendar::DAY,WEEK,MONTH,YEAR,
	 * and in addition also dst.
	 * e.g. the diff_units of calendar::DAY over summer->winter shift is 1, remainder is 0,
	 * even if the number of hours during those days are 23 and 25 summer and winter transition respectively
	 *
	 * \sa calendar::add
	 *
	 * \return (t2-t1)/deltaT, and remainder, where deltaT could be calendar units DAY,WEEK,MONTH,YEAR
	 */
	timespan_t diff_units(time_t t1, time_t t2, timespan_t deltaT, timespan_t &remainder) const ;
	///< diff_units discarding remainder, \sa diff_units
	timespan_t diff_units(time_t t1, time_t t2, timespan_t deltaT) const {
		timespan_t ignore;
		return diff_units(t1,t2,deltaT,ignore);
	}
};

namespace zone{
	inline timespan_t tz_table::dst_offset(time_t t) const {
		if(!is_dst()) return  timespan_t(0);
		auto year=cal_t::utc_year(t);
		if(year-start_year >= (int) dst.size()) return timespan_t(0);
		auto s=dst_start(year);
		auto e=dst_end(year);
		return (s<e?(t>=s&&t<e):(t<e || t>=s))?dt[year-start_year]:timespan_t(0);
	}
}

}
