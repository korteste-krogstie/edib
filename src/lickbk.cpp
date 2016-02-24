#include<algorithm>
#include<exception>
#include<iostream>
#include<vector>
#include<map>
#include<stdexcept>
#include<cstdlib>
#include<cstring>
#include<cstdint>
#include<chrono>
#include<string>

#include<stdlib.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>

#include<cog/cog.hpp>
#include<cog/ascii/reg.hpp>
#include<cog/ascii/misc.hpp>
#include<cog/ascii/alpha.hpp>

#include<cdf/cdf.hpp>
#include<utc.hpp>
#include<utm.hpp>


namespace chr=std::chrono;
using idx_t=std::size_t;

namespace trait{
template<class tag>
struct
	key_regex
{ std::string operator()() const { return ""; } };

template<class tag>
struct
	val_regex
{ std::string operator()() const { return ""; } };

template<class tag>
struct
	top_level
{ constexpr bool operator()() const { return true; } };

}

template<class tag>
std::string key_regex(){ return trait::key_regex<tag>{}(); }

template<class tag>
std::string val_regex(){ return trait::val_regex<tag>{}(); }

template<class tag>
constexpr bool top_level(){ return trait::top_level<tag>{}(); }

namespace tag{
	struct 
		element
	{};

	template<std::size_t idx>
	struct 
		member
	{
		constexpr std::size_t operator()() const { return idx; }
	};
}

namespace guts{
	template<class tag>
	struct
		member_idx;
	template<std::size_t idx>
	struct
		member_idx<tag::member<idx>>
	{ constexpr std::size_t operator()() const { return idx; } };
}
template<class tag>
constexpr std::size_t member_idx=guts::member_idx<tag>{}();

namespace guts{
	constexpr std::size_t idx_offs=__COUNTER__+1;
}

#define MEMBER(id,key_regex_str,val_regex_str)\
	namespace tag{\
		using id=tag::member<__COUNTER__-guts::idx_offs>;\
	}\
	namespace trait{\
		template<>\
		struct key_regex<tag::id>{\
			std::string operator()() const { return key_regex_str; }\
		};\
		template<>\
		struct val_regex<tag::id>{\
			std::string operator()() const { return val_regex_str; }\
		};\
	}
MEMBER(hierarchy,"hierarchy","\"[^\"]+\"")
MEMBER(timestamp,"timestamp","[0-9]+")
MEMBER(latitude,"longitude","[0-9]+\\.[0-9]+")
MEMBER(salt_timestamp,"salt_timestamp","[0-9]+")
MEMBER(longitude,"latitude","[0-9]+\\.[0-9]+")
MEMBER(id,"id","\"[0-9]+\"")
MEMBER(accuracy,"accuracy","[0-9]+\\.[0-9]+")
#undef MEMBER

#define ATTR(id)\
	namespace tag{\
		using id=tag::member<__COUNTER__-guts::idx_offs>;\
	}\
	namespace trait{\
		template<>\
		struct top_level<tag::id>{\
			constexpr bool operator()(){\
				return false;\
			}\
		};\
	}
ATTR(building)
ATTR(floor)
#undef ATTR

constexpr std::size_t member_num=__COUNTER__-guts::idx_offs;

namespace guts{

	template<class integer>
	struct
		member_tag_tf
	{ using output=tag::member<integer{}()>; };

}
using tags=seq::transform<seq::integer_range<0,member_num>,guts::member_tag_tf>;

namespace guts{
	template<class... tag>
	constexpr std::array<bool,member_num>
		mk_top_level_tab(const seq::sequence<tag...>&)
	{ return { top_level<tag>()... }; }
}
constexpr std::array<bool,member_num> top_level_tab = guts::mk_top_level_tab(tags{});

namespace guts{

	template<class... tag>
	std::array<std::string,member_num>
		mk_key_regex_tab(const seq::sequence<tag...>&) 
	{ return { key_regex<tag>()... }; }

}
const std::array<std::string,member_num> key_regex_tab = guts::mk_key_regex_tab(tags{}); 

namespace guts{

	template<class... tag>
	std::array<std::string,member_num>
		mk_val_regex_tab(const seq::sequence<tag...>&) 
	{ return { val_regex<tag>()... }; }

}
const std::array<std::string,member_num> val_regex_tab = guts::mk_val_regex_tab(tags{}); 

namespace guts{

	template<std::size_t... member_idx>
	std::array<std::size_t,member_num>
		mk_sem_beg_idx(const seq::sequence<tag::member<member_idx>...>&)
	{ return { ((1+member_num)+1+member_idx)... }; }

}
const std::array<std::size_t,member_num> sem_beg_idx = guts::mk_sem_beg_idx(tags{});

namespace guts{

	template<std::size_t... member_idx>
	std::array<std::size_t,member_num>
		mk_sem_end_idx(const seq::sequence<tag::member<member_idx>...>&)
	{ return { (1+member_idx)... }; }

}
const std::array<std::size_t,member_num> sem_end_idx = guts::mk_sem_end_idx(tags{});

namespace guts{

	template<class tag>
	cog::nfa_tt<idx_t>
		val_nfa(cog::nfa_tt<idx_t>& term_nfa)
	{
		const auto& lit=cog::ascii::lit<idx_t>;
		const auto& reg=cog::ascii::reg<idx_t>;
		auto nfa=reg(val_regex_tab[::member_idx<tag>]);

		for(auto del_idx:nfa.rho[nfa.init].source_ref)
			nfa.del[del_idx].sem.insert(sem_beg_idx[::member_idx<tag::nfa>]);

		for(auto del_idx:term_nfa.rho[term_nfa.init].source_ref)
			term_nfa.del[del_idx].sem.insert(sem_end_idx[::member_idx<tag::nfa>]);

		return std::move(nfa);
	}

	template<>
	cog::nfa_tt<idx_t>
		val_nfa<tag::hierarchy>(cog::nfa_tt<idx_t>& term_nfa)
	{
		const auto& lit=cog::ascii::lit<idx_t>;
		const auto& reg=cog::ascii::reg<idx_t>;

		auto area_nfa=reg("[^>]+>");
		for(auto del_idx:area_nfa.rho[area_nfa.init].source_ref)
			area_nfa.del[del_idx].sem.insert(sem_beg_idx[tag::hierarchy{}()]);

		auto building_nfa=reg("[^{}()]+>");
		for(auto del_idx:building_nfa.rho[building_nfa.init].source_ref)
			building_nfa.del[del_idx].sem.insert(sem_beg_idx[tag::building{}()]);

		for(auto rho_idx:building_nfa.term)
			for(auto del_idx:building_nfa.rho[rho_idx].source_ref)
				building_nfa.del[del_idx].sem.insert(sem_end_idx[tag::building{}()]);

		auto floor_nfa=reg("[0-9]+");

		for(auto del_idx:floor_nfa.rho[floor_nfa.init].source_ref)
			floor_nfa.del[del_idx].sem.insert(sem_beg_idx[tag::floor{}()]);
		
		auto tail_nfa=reg("[^\"]+\"");
		for(auto del_idx:tail_nfa.rho[tail_nfa.init].source_ref)
			tail_nfa.del[del_idx].sem.insert(sem_end_idx[tag::floor{}()]);


		for(auto del_idx:term_nfa.rho[term_nfa.init].source_ref)
			term_nfa.del[del_idx].sem.insert(sem_end_idx[tag::hierarchy{}()]);

		return cog::con(std::move(area_nfa),std::move(building_nfa),std::move(floor_nfa),std::move(tail_nfa));
	}

	template<class... tag>
	constexpr std::array<cog::nfa_tt<idx_t>(*)(cog::nfa_tt<idx_t>&),member_num>
		mk_val_nfa_tab(const seq::sequence<tag...>&)
	{ return { val_nfa<tag>... }; }
}

const std::array<cog::nfa_tt<idx_t>(*)(cog::nfa_tt<idx_t>&),member_num>
	val_nfa_tab = guts::mk_val_nfa_tab(tags{});

cog::nfa_tt<idx_t>
	elem_nfa()
{
	const auto& lit=cog::ascii::lit<idx_t>;
	const auto& reg=cog::ascii::reg<idx_t>;
	auto nfa = lit('{');
	for(idx_t member_idx=0;member_idx<member_num;++member_idx){
		if(!top_level_tab[member_idx])
			continue;

		cog::nfa_tt<idx_t> term_nfa = [&]{
			if(member_idx+1==member_num)
				return lit('}');
			else
				return reg(", ");
		}();

		auto key_nfa = reg(std::string("\"")+key_regex_tab[member_idx]+std::string("\""));
		auto pair_sep_nfa=reg(": ");
		auto val_nfa = val_nfa_tab[member_idx](term_nfa);
		nfa=cog::con(nfa,key_nfa,pair_sep_nfa,val_nfa,term_nfa);
	}
	return nfa;
}

constexpr std::size_t 
	elem_sem_beg_idx=member_num+1,
	elem_sem_end_idx=0;

cog::nfa_tt<idx_t>
	file_nfa()
{
	const auto& lit=cog::ascii::lit<idx_t>;
	const auto& reg=cog::ascii::reg<idx_t>;

	auto file_beg_nfa=lit('[');

	for(auto rho_idx:file_beg_nfa.term)
		for(auto del_idx:file_beg_nfa.rho[rho_idx].target_ref)
			file_beg_nfa.del[del_idx].sem = { elem_sem_beg_idx };

	auto elem_sep_nfa = reg(",");

	for(auto del_idx:elem_sep_nfa.rho[elem_sep_nfa.init].source_ref)
		elem_sep_nfa.del[del_idx].sem.insert(elem_sem_end_idx);

	for(auto rho_idx:elem_sep_nfa.term)
		for(auto del_idx:elem_sep_nfa.rho[rho_idx].target_ref)
			elem_sep_nfa.del[del_idx].sem.insert(elem_sem_beg_idx);

	auto file_end_nfa=lit(']');

	for(auto del_idx:file_end_nfa.rho[file_end_nfa.init].source_ref)
		file_end_nfa.del[del_idx].sem = { elem_sem_end_idx };

	auto elem_nfa=::elem_nfa();
	return cog::con(
		file_beg_nfa,
		elem_nfa,
		cog::rep(cog::con(elem_sep_nfa,elem_nfa)),
		file_end_nfa,
		cog::opt(lit('\n'))
	);
}

namespace guts{

	template<class tongue_t>
	void
		act_elem_beg(tongue_t& tongue,char* data,std::size_t data_idx)
	{ tongue.beg(tag::element{},data,data_idx); }

	template<std::size_t member_idx,class tongue_t>
	void
		act_member_beg(tongue_t& tongue,char* data,std::size_t data_idx)
	{ tongue.beg(tag::member<member_idx>{},data,data_idx); }

	template<class tongue_t>
	void
		act_elem_end(tongue_t& tongue,char* data,std::size_t data_idx)
	{ tongue.end(tag::element{},data,data_idx); }

	template<std::size_t member_idx,class tongue_t>
	void
		act_member_end(tongue_t& tongue,char* data,std::size_t data_idx)
	{ tongue.end(tag::member<member_idx>{},data,data_idx); }

	template<class tongue_t,std::size_t... member_idx>
	static constexpr std::array<void(*)(tongue_t&,char*,std::size_t),member_num*2+2>
		mk_act_tab(const seq::sequence<tag::member<member_idx>...>&)
	{
		return std::array<void(*)(tongue_t&,char*,std::size_t),member_num*2+2>{
			act_elem_end<tongue_t>,act_member_end<member_idx,tongue_t>..., 
			act_elem_beg<tongue_t>,act_member_beg<member_idx,tongue_t>...
		};
	}

}

struct mouth_t{
	std::vector<std::vector<idx_t>> del_tab;
	std::vector<bool> 
		term_tab;
	idx_t rho_init,undef_idx;
	std::vector<std::vector<std::vector<idx_t>>> del_sem_tab;
	std::vector<idx_t> ascii_map;

	mouth_t(){
		cog::dfa_tt<idx_t> dfa;
		std::map<idx_t,std::set<idx_t>> inv_min_sym;
		std::vector<idx_t> inv_min_al;
		std::tie(dfa,std::ignore) = cog::to_dfa(file_nfa());
		std::tie(dfa,std::ignore) = cog::min_rho(dfa);
		std::tie(dfa,inv_min_sym) = cog::min_sym(dfa);
		std::tie(dfa,inv_min_al) = cog::min_al(dfa);
		ascii_map = cog::sym_map(cog::ascii::alpha<idx_t>,inv_min_sym,inv_min_al);
		rho_init=dfa.init;
		undef_idx=dfa.undef;
		std::tie(del_tab,term_tab) = cog::gen::tab(dfa);

		del_sem_tab = std::vector<std::vector<std::vector<idx_t>>>(dfa.rho.size(),std::vector<std::vector<idx_t>>(dfa.al.sym_num));
		for(idx_t del_idx=0;del_idx<dfa.del.size();++del_idx){
			const auto& del=dfa.del[del_idx];
			for(auto sem_idx:del.sem)
				del_sem_tab[del.source][del.sym].push_back(sem_idx);
		}

	}

	template<class tongue_t>
	void
		operator()(tongue_t& tongue,char* data,std::size_t size)
	{
		static constexpr std::array<void(*)(tongue_t&,char*,std::size_t),2*member_num+2>
			act_tab = guts::mk_act_tab<tongue_t>(tags{});
		std::size_t data_idx=0;
		idx_t rho_idx=rho_init,sym_idx=undef_idx;
		std::cout<<"starting lick...\n";
		auto start=chr::high_resolution_clock::now();
		for(
			sym_idx=ascii_map[data[data_idx]];
			data_idx<size&&sym_idx!=undef_idx&&rho_idx!=undef_idx;
			sym_idx=ascii_map[data[++data_idx]]
		){ 
			for(auto act_idx:del_sem_tab[rho_idx][sym_idx])
				act_tab[act_idx](tongue,data,data_idx);
			rho_idx=del_tab[rho_idx][sym_idx];
		}
		auto stop=chr::high_resolution_clock::now();

		if(rho_idx==undef_idx){
			std::cout<<"jammed at idx: "<<data_idx-1<<"\n";
			std::cout<<"\t";
			for(idx_t i=(std::max(data_idx,std::size_t(6))-6);i<std::min(data_idx+6,size);++i)
				std::cout<<data[i];
			std::cout<<"\n\t";
			for(idx_t i=(std::max(data_idx,std::size_t(6))-6);i<std::min(data_idx+6,size);++i)
				if(i+1==data_idx)
					std::cout<<"^";
				else
					std::cout<<" ";
			std::cout<<"\n";
		}
		else if(sym_idx==undef_idx)
			std::cout<<"fail, unknown input at "<<data_idx<<" = "<<(int)(data[data_idx])<<"\n";
		else if(data_idx!=size)
			std::cout<<"fail, premature exit at "<<data_idx<<" = "<<(int)(data[data_idx])<<"\n";
		else if(!term_tab[rho_idx])
			std::cout<<"fail, not term\n";
		else{
			std::cout
				<<"lick complete\n"
				<<"\tinterval = "<<chr::duration_cast<chr::milliseconds>(stop-start).count()<<"ms\n";
		}
	}
};

struct
	coord_t
{
	utm::point_t pos;
	utc::time_t time;
};

struct 
	trace_t
{
	utc::time_t salt;
	uint32_t id;
	utc::time_t min=utc::max_time,max=utc::min_time;
	uint64_t num=0;
	std::vector<coord_t> coord;
};

struct
	tongue_t
{
	std::vector<trace_t>& trace;
	std::map<std::pair<utc::time_t,uint32_t>,idx_t> key_map;
	std::size_t elem_idx=0,aux_idx=0;

	utc::cal_t cal;

	uint32_t id_val;
	utc::time_t salt_timestamp_val,timestamp_val;

	void beg(tag::element,char* data,std::size_t idx){}
	void end(tag::element,char* data,std::size_t idx){
/*
		std::cout
			<<"end elem "<<elem_idx++
			<<"\n\ttimestamp: "<<cal.to_string(timestamp_val)
			<<"\n\tsalt: "<<cal.to_string(salt_timestamp_val)
			<<"\n\tid: "<<id_val
			<<"\n";
*/
		std::size_t trace_idx=trace.size();
		auto it=key_map.insert({{salt_timestamp_val,id_val},trace_idx});
		if(!it.second){
			trace_idx=it.first->second;
			trace[trace_idx].min=std::min(timestamp_val,trace[trace_idx].min);
			if(trace[trace_idx].max < timestamp_val){
				trace[trace_idx].max = timestamp_val;
				trace[trace_idx].point.push_back(timestamp_val);
			}else{
				trace[trace_idx].point.insert(
					std::lower_bound(trace[trace_idx].point.begin(),trace[trace_idx].point.end(),timestamp_val),
					timestamp_val
				);
			}
			++trace[trace_idx].num;
		}else{
			trace.push_back(trace_t{salt_timestamp_val,id_val,timestamp_val,timestamp_val,1,{timestamp_val}});
		}
	}

	void beg(tag::id,char* data,std::size_t idx){
		aux_idx=idx;
	}
	void end(tag::id,char* data,std::size_t idx){
		char backup=data[idx];
		data[idx]='\0';
		char* ptr_end;
		id_val=std::strtoul(data+aux_idx+2,&ptr_end,10); //TODO: check
		data[idx]=backup;
	}

	void beg(tag::salt_timestamp,char* data,std::size_t idx){
		aux_idx=idx;
	}
	void end(tag::salt_timestamp,char* data,std::size_t idx){
		char backup=data[idx];
		data[idx]='\0';
		char* ptr_end;
		salt_timestamp_val=std::strtoll(data+aux_idx,&ptr_end,10);
		data[idx]=backup;
	}

	void beg(tag::timestamp,char* data,std::size_t idx){
		aux_idx=idx;
	}
	void end(tag::timestamp,char* data,std::size_t idx){
		char backup=data[idx];
		data[idx]='\0';
		char* ptr_end;
		timestamp_val=std::strtoll(data+aux_idx,&ptr_end,10);
		data[idx]=backup;
	}

	void end(tag::latitude,char* data,std::size_t idx){

	}
	void beg(tag::latitude,char* data,std::size_t idx){

	}

	void end(tag::longitude,char* data,std::size_t idx){

	}
	void beg(tag::longitude,char* data,std::size_t idx){

	}

	template<class tag>
	void beg(tag,char*,std::size_t){}
	template<class tag>
	void end(tag,char*,std::size_t){}

};

int
	main(int argc,char** argv)
{
	try{
		if(argc<3)
			throw std::runtime_error("usage: lick [shm_in] [cdf_hist_out] [cdf_trace_out]?");

		std::string memname=argv[1];
		std::cout<<"licking "<<memname<<std::endl;
		std::cout<<"opening shm...\n";
		int fd=shm_open(memname.c_str(),/*O_RDONLY*/O_RDWR,S_IRUSR);
		if(fd<0)
			throw std::runtime_error("shm_open fail");
		std::cout<<"done...\n";


		char* data=nullptr;
		std::size_t size;
		{
			struct stat buf;
			std::cout<<"computing size...\n";
			if(fstat(fd,&buf)<0){
				shm_unlink(memname.c_str());
				throw std::runtime_error("fstat fail");
			}
			size=buf.st_size;
			std::cout<<"done... size = "<<size<<"b.\n";
			if(size==0){
				shm_unlink(memname.c_str());
				throw std::runtime_error("empty file");
			}
			data=(char*)(mmap(nullptr,size,/*PROT_READ*/PROT_READ|PROT_WRITE,MAP_SHARED,fd,0));
			if(!data)
				throw std::runtime_error("mmap failed");
		}

		mouth_t mouth;
		std::vector<trace_t> trace;
		tongue_t tongue{trace};
		mouth(tongue,data,size);
		std::cout<<"elem_num = "<<tongue.elem_idx<<"\n";

		munmap(data,size);
		close(fd);
/*		{
			using val_t=std::pair<int64_t,uint32_t>;
			std::vector<val_t> trace_len;
			
			std::vector<val_t>::iterator iterator;
			for(std::size_t i=0;i<trace.size();++i){
				val_t val{trace[i].max-trace[i].min,1};
				auto it=std::lower_bound(
					trace_len.begin(),
					trace_len.end(),
					val,
					[](const val_t& lhs,const val_t& rhs){
						return lhs.first<rhs.first;
					}
				);
				if(it==trace_len.end()||it->first!=val.first)
					trace_len.insert(it,val);
				else
					++it->second;
			}

			std::string cdf_filename=argv[2];
			cdf::file_t file=cdf::mk_file(cdf_filename);

			std::size_t trace_len_num=trace_len.size();

			std::cout<<"trace len num = "<<trace_len_num<<"\n";
			cdf::dim_t idx_dim(file,"idx_dim",trace_len_num);
			cdf::var_tt<int64_t,1> 
				sec_var(file,{&idx_dim},"sec_var");
			cdf::var_tt<uint32_t,1>
				num_var(file,{&idx_dim},"num_var");

			cdf::enddef(file);

			cdf::put(sec_var,{0},{trace_len_num},{1},{sizeof(val_t)},&trace_len.data()->first);
			cdf::put(num_var,{0},{trace_len_num},{1},{sizeof(val_t)},&trace_len.data()->second);
			
		}
*/
		{
			using val_t=std::pair<int64_t,uint32_t>;
			std::vector<val_t> point_interval;
			
			std::vector<val_t>::iterator iterator;
			for(std::size_t i=0;i<trace.size();++i){
				for(std::size_t j=0;j<trace[i].point.size();++j){
					if(j+1==trace[i].point.size())
						continue;
					auto& P=trace[i].point;
					auto interval = P[j+1]-P[j];
					auto it=std::lower_bound(
						point_interval.begin(),
						point_interval.end(),
						val_t{interval,1},
						[](const val_t& v0,const val_t& v1){
							return v0.first<v1.first;
						}
					);
					if(it==point_interval.end()||it->first!=interval)
						point_interval.insert(it,{interval,1});
					else
						++it->second;
				}
			}

			std::string cdf_filename=argv[2];
			cdf::file_t file=cdf::mk_file(cdf_filename);

			std::size_t point_interval_num=point_interval.size();

			std::cout<<"point interval num = "<<point_interval_num<<"\n";

			cdf::dim_t idx_dim(file,"idx_dim",point_interval_num);
			cdf::var_tt<int64_t,1> sec_var(file,{&idx_dim},"sec_var");
			cdf::var_tt<uint32_t,1> num_var(file,{&idx_dim},"num_var");
			cdf::enddef(file);
			cdf::put(sec_var,{0},{point_interval_num},{1},{sizeof(val_t)},&point_interval.data()->first);
			cdf::put(num_var,{0},{point_interval_num},{1},{sizeof(val_t)},&point_interval.data()->second);

		}

		if(argc==4){
			std::string cdf_filename=argv[3];
			cdf::file_t file=cdf::mk_file(cdf_filename);

			std::size_t trace_num=trace.size();

			std::cout<<"trace num = "<<trace_num<<"\n";
			cdf::dim_t trace_dim(file,"trace_dim",trace_num);

			cdf::var_tt<int64_t,1> 
				salt_var(file,{&trace_dim},"salt_var"),
				min_var(file,{&trace_dim},"min_var"),
				max_var(file,{&trace_dim},"max_var");
			cdf::var_tt<uint32_t,1>
				id_var(file,{&trace_dim},"id_var");
			cdf::var_tt<uint64_t,1>
				num_var(file,{&trace_dim},"num_var");

			cdf::enddef(file);

			cdf::put(
				num_var,
				{std::size_t(0)},
				{trace_num},
				{(std::ptrdiff_t)(1)},
				{(std::ptrdiff_t)(sizeof(trace_t))},
				&trace.data()->num
			);

			cdf::put(
				salt_var,
				{std::size_t(0)},
				{trace_num},
				{(std::ptrdiff_t)(1)},
				{(std::ptrdiff_t)(sizeof(trace_t))},
				&trace.data()->salt
			);

			cdf::put(
				min_var,
				{std::size_t(0)},
				{trace_num},
				{(std::ptrdiff_t)(1)},
				{(std::ptrdiff_t)(sizeof(trace_t))},
				&trace.data()->min
			);

			cdf::put(
				max_var,
				{std::size_t(0)},
				{trace_num},
				{(std::ptrdiff_t)(1)},
				{(std::ptrdiff_t)(sizeof(trace_t))},
				&trace.data()->max
			);

			cdf::put(
				id_var,
				{std::size_t(0)},
				{trace_num},
				{(std::ptrdiff_t)(1)},
				{(std::ptrdiff_t)(sizeof(trace_t))},
				&trace.data()->id
			);
		}
	}catch(const std::exception& e){
		std::cerr<<"error: "<<e.what()<<std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
