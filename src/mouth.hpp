#pragma once
#include<array>
#include<cmath>
#include<iostream>
#include<map>
#include<set>
#include<tuple>
#include<vector>
#include<cog/dfa.hpp>
#include<cog/nfa.hpp>
#include<cog/ascii/reg.hpp>
#include<cog/gen/tab.hpp>
#include<lick.hpp>
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
	std::string filename;
	int fd=-1;
	char* data=nullptr;
	std::size_t size;
	std::vector<std::vector<idx_t>> del_tab;
	std::vector<bool> 
		term_tab;
	idx_t rho_init,undef_idx;
	std::vector<std::vector<std::vector<idx_t>>> del_sem_tab;
	std::vector<idx_t> ascii_map;

	mouth_t(int argc,char** argv)
	{
		if(argc<2)
			throw std::runtime_error("usage: lick shm_name ...");
		filename=argv[1];
		{
			std::cout<<"opening shm at "<<filename<<std::endl;
			fd=shm_open(filename.c_str(),/*O_RDONLY*/O_RDWR,S_IRUSR);
			if(fd<0)
				throw std::runtime_error("shm_open fail");
			std::cout<<"done...\n";

			{
				struct stat buf;
				std::cout<<"computing size...\n";
				if(fstat(fd,&buf)<0){
					shm_unlink(filename.c_str());
					throw std::runtime_error("fstat fail");
				}
				size=buf.st_size;
				std::cout<<"done... size = "<<size<<"b.\n";
				if(size==0){
					shm_unlink(filename.c_str());
					throw std::runtime_error("empty file");
				}
				data=(char*)(mmap(nullptr,size,/*PROT_READ*/PROT_READ|PROT_WRITE,MAP_SHARED,fd,0));
				if(!data)
					throw std::runtime_error("mmap failed");
			}

		}
		
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
	~mouth_t(){
		if(data)
			munmap(data,size);
		if(fd>=0)
			close(fd);
	}

	template<class tongue_t>
	void
		operator()(tongue_t& tongue)
	{
		static constexpr std::array<void(*)(tongue_t&,char*,std::size_t),2*member_num+2>
			act_tab = guts::mk_act_tab<tongue_t>(tags{});
		std::size_t data_idx=0;
		idx_t rho_idx=rho_init,sym_idx=undef_idx;
		std::cout<<"starting lick...\n";
		tongue.beg(data,data_idx);
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
			tongue.end(data,data_idx);
		}
	}
};
