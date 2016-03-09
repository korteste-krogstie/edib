#pragma once
#include<array>
#include<chrono>
#include<string>
#include<utility>
#include<cog/fa.hpp>
#include<cog/dfa.hpp>
#include<cog/nfa.hpp>
#include<cog/ascii/reg.hpp>
#include<seq/seq.hpp>

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
	{ constexpr std::size_t operator()() const { return idx; } };
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
