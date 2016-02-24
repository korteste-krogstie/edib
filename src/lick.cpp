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
	constexpr std::size_t member_idx_offs=__COUNTER__+1;
}

#define MEMBER(id,key_regex_str,val_regex_str)\
	namespace tag{\
		using id=tag::member<__COUNTER__-guts::member_idx_offs>;\
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
		using id=tag::member<__COUNTER__-guts::member_idx_offs>;\
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

constexpr std::size_t member_num=__COUNTER__-guts::member_idx_offs;

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
		auto nfa=reg(val_regex_tab[tag{}()]);

		for(auto del_idx:nfa.rho[nfa.init].source_ref)
			nfa.del[del_idx].sem.insert(sem_beg_idx[tag{}()]);

		for(auto del_idx:term_nfa.rho[term_nfa.init].source_ref)
			term_nfa.del[del_idx].sem.insert(sem_end_idx[tag{}()]);

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

		auto building_nfa=reg("[^>]+>");
		for(auto del_idx:building_nfa.rho[building_nfa.init].source_ref)
			building_nfa.del[del_idx].sem.insert(sem_beg_idx[tag::building{}()]);

		for(auto rho_idx:building_nfa.term)
			for(auto del_idx:building_nfa.rho[rho_idx].target_ref)
				building_nfa.del[del_idx].sem.insert(sem_end_idx[tag::building{}()]);

		auto floor_nfa=reg("[^\"]+\"");
		for(auto del_idx:floor_nfa.rho[floor_nfa.init].source_ref)
			floor_nfa.del[del_idx].sem.insert(sem_beg_idx[tag::floor{}()]);
		
		for(auto rho_idx:floor_nfa.term)
			for(auto del_idx:floor_nfa.rho[rho_idx].target_ref)
				floor_nfa.del[del_idx].sem.insert(sem_end_idx[tag::floor{}()]);

		for(auto del_idx:term_nfa.rho[term_nfa.init].source_ref)
			term_nfa.del[del_idx].sem.insert(sem_end_idx[tag::hierarchy{}()]);

		return cog::con(std::move(area_nfa),std::move(building_nfa),std::move(floor_nfa));
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
			if(member_idx+3==member_num)
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

struct tongue_t{
	std::map<std::string,std::set<std::string>> building_map;

	std::size_t 
		hier_idx,
		build_idx,build_num,
		floor_idx,floor_num;

	void beg(tag::hierarchy,char*,std::size_t idx){
		hier_idx=idx;
	}
	void end(tag::hierarchy,char* data,std::size_t idx){
		auto& building=building_map[std::string(data+build_idx,build_num)];
		building.insert(std::string(data+floor_idx,floor_num));
	}

	void beg(tag::building,char*,std::size_t idx){
		build_idx=idx;
	}
	void end(tag::building,char* data,std::size_t idx){
		build_num=idx-build_idx;
	}

	void beg(tag::floor,char*,std::size_t idx){
		floor_idx=idx;
	}
	void end(tag::floor,char* data,std::size_t idx){
		floor_num=idx-floor_idx;
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
		if(argc!=3)
			throw std::runtime_error("usage: lick [shm_in] [build_out]");

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
		tongue_t tongue;
		mouth(tongue,data,size);

		munmap(data,size);
		close(fd);

		std::ofstream file(argv[2]);
		for(auto& building_pair:tongue.building_map){
			file<<building_pair.first<<"\n";
			for(auto& floor:building_pair.second)
				file<<"\t"<<floor<<"\n";
		}
	}catch(const std::exception& e){
		std::cerr<<"error: "<<e.what()<<std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
