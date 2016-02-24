#include<algorithm>
#include<array>
#include<chrono>
#include<cstdint>
#include<cstdlib>
#include<cstring>
#include<exception>
#include<iostream>
#include<stdexcept>
#include<random>

#include<sys/mman.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>

#include<cog/cog.hpp>
#include<cog/gen/tab.hpp>
#include<cog/ascii/reg.hpp>
#include<cdf/cdf.hpp>
#include<tio/tio.hpp>

#include<utc.hpp>
#include<utm.hpp>

namespace chr=std::chrono;
using namespace std::literals;

using idx_t=unsigned short;

static constexpr std::size_t sig_dig=4;

struct point_t{
	utc::time_t time;
	utm::point_t point;
};

uint32_t max_id_val=0;
//inngang paa stripa
constexpr double abs_max_x=1000.0;
constexpr double abs_max_y=1500.0;
constexpr double max_quad=300.0f;
constexpr utm::point_t origo{270647.954,7040417.779,33};

inline bool in_range(utm::point_t p){
	return (std::fabs(p.x)<=abs_max_x)&&(std::fabs(p.y)<=abs_max_y);
}

//min_time = 2014 / 08 / 31
//max_time = 2014 / 11/ 01
utc::time_t min_time=1409511777;
utc::time_t max_time=1414872177;

inline bool in_range(utc::time_t t){
	return t>=min_time && t<=max_time;
}

int
	main(int argc,char** argv)
{
	try{
		if(argc!=2)
			throw std::runtime_error("usage: slurp [memfile]");

		std::string memname=argv[1];
		std::cout<<"slurping from "<<memname<<std::endl;

		std::map<std::pair<utc::time_t,uint16_t>,idx_t> key_map;

		std::vector<std::vector<int16_t>> xpos,ypos;
		std::vector<std::vector<int64_t>> tpos;

		std::cout<<"opening shm...\n";
		int fd=shm_open(memname.c_str(),/*O_RDONLY*/O_RDWR,S_IRUSR);
		if(fd<0)
			throw std::runtime_error("shm_open fail");
		std::cout<<"done...\n";

		
		char* data=nullptr;
		std::size_t size;
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
		
		std::size_t elem_aux_idx=0;
		std::vector<std::vector<idx_t>> del_tab;
		std::vector<bool> 
			term_tab;
		idx_t rho_init,rho_undef;

		std::vector<std::vector<std::vector<idx_t>>> del_sem_tab;

		std::vector<idx_t> ascii_map;

		std::vector<std::function<void(char*,std::size_t)>> act;

		utc::cal_t cal;	
		utc::time_t time_val,salt_val;
		uint32_t id_val;
		double lon_val,lat_val;

		std::size_t aux_idx;
		std::size_t elem_idx=0,pos_filter_num=0,tpos_filter_num=0,prox_filter_num=0;

		{
			const auto& lit=cog::ascii::lit<idx_t>;
			const auto& reg=cog::ascii::reg<idx_t>;

			idx_t sem_idx=0;

			const idx_t
				elem_end_idx=sem_idx++,
				elem_beg_idx=sem_idx++;

			act.push_back([&](char* d,std::size_t di){
//				points.emplace_back(point_t{time_val,utm::to_utm(lat_val,lon_val,33)-origo});

				auto p=utm::to_utm(lat_val,lon_val,33)-origo;
				/*
				std::cout
					<<elem_idx<<":"
					<<"\n\ttpos: "<<cal.to_string(time_val)<<", "
					<<"\n\txpos: "<<p.x<<", "
					<<"\n\typos: "<<p.y<<", "
					<<"\n\tid: "<<id_val<<", "
					<<"\n\tsalt: "<<cal.to_string(salt_val)
					<<"\n";
*/
				if(!in_range(time_val)){
					++tpos_filter_num;
					return;
				}
				if(!in_range(p)){
					++pos_filter_num;
					return;
				}
				idx_t idx=tpos.size();
				++elem_idx;
				{
					auto it=key_map.insert({{salt_val,id_val},idx});
					if(!it.second)
						idx=it.first->second;
					else{
						xpos.emplace_back();
						ypos.emplace_back();
						tpos.emplace_back();
					}
				}

				if(!xpos[idx].empty()){
					auto q=utm::quad(p,utm::point_t{(double)(xpos[idx].back()),(double)(ypos[idx].back())});
					if(q < max_quad){
						++prox_filter_num;
						return;
					}
				}
				xpos[idx].push_back(p.x);
				ypos[idx].push_back(p.y);
				tpos[idx].push_back(time_val);
			});
			act.push_back([&](char* d,std::size_t idx){
				elem_aux_idx=idx;
//				std::cout<<"begin elem: "<<std::string(d+idx,8)<<"\n";
			});

			std::vector<std::string> 
				mem_key,
				mem_val;

			std::vector<idx_t>
				mem_beg_idx,
				mem_end_idx;

			std::size_t mem_idx=0;

			auto def_mem = [&](
				std::string key,
				std::string val,
				std::function<void(char*,std::size_t)> end=std::function<void(char*,std::size_t)>(),
				std::function<void(char*,std::size_t)> beg=std::function<void(char*,std::size_t)>()
			){
				mem_key.push_back(key);
				mem_val.push_back(val);
				mem_end_idx.push_back(sem_idx++);
				mem_beg_idx.push_back(sem_idx++);

				act.push_back(end);
				act.push_back(beg);
			};

			const std::size_t hier_idx = mem_idx++;
			def_mem("hierarchy","\"[^\"]+\"");

			const std::size_t time_idx = mem_idx++;
			def_mem(
				"timestamp",
				"[0-9]+",
				[&](char* data,std::size_t num){
					char backup=data[num];
					data[num]='\0';
					time_val=std::strtoll(data+aux_idx,nullptr,10);
					data[num]=backup;
				},
				[&](char* data,std::size_t num){
					aux_idx=num;
				}
			);

			const std::size_t lati_idx = mem_idx++;
			def_mem(
				"longitude",
				"[0-9]+\\.[0-9]+",
				[&](char* data,std::size_t num){
					char backup=data[num];
					data[num]='\0';
					lat_val=std::strtof(data+aux_idx,nullptr);
					data[num]=backup;
					lat_val=std::trunc(lat_val*std::pow(10,sig_dig))/std::pow(10,sig_dig);
				},
				[&](char* data,std::size_t num){
					aux_idx=num;
				}
			);

			const std::size_t salt_idx = mem_idx++;
			def_mem(
				"salt_timestamp",
				"[0-9]+",
				[&](char* data,std::size_t num){
					char backup=data[num];
					data[num]='\0';
					salt_val=std::strtoll(data+aux_idx,nullptr,10);
					data[num]=backup;
				},
				[&](char* data,std::size_t num){
					aux_idx=num;
				}
			);

			const std::size_t longi_idx = mem_idx++;
			def_mem(
				"latitude",
				"[0-9]+\\.[0-9]+",
				[&](char* data,std::size_t num){
					char backup=data[num];
					data[num]='\0';
					lon_val=std::strtof(data+aux_idx,nullptr);
					data[num]=backup;
					lon_val=std::trunc(lon_val*std::pow(10,sig_dig))/std::pow(10,sig_dig);
				},
				[&](char* data,std::size_t num){
					aux_idx=num;
				}
			);

			const std::size_t id_idx = mem_idx++;
			def_mem(
				"id",
				"\"[0-9]+\"",
				[&](char* data,std::size_t idx){
					char backup=data[idx];
					data[idx]='\0';
					id_val=std::strtoul(data+aux_idx+2,nullptr,10); //TODO: check
					data[idx]=backup;
					max_id_val=std::max(max_id_val,id_val);
				},
				[&](char* data,std::size_t idx){
					aux_idx=idx;
				}
			);

			const std::size_t acc_idx = mem_idx++;
			def_mem("accuracy","[0-9]+\\.[0-9]+");

			const std::size_t mem_num=mem_idx;

			auto elem_nfa = [&]{
				auto nfa = lit('{');

				for(std::size_t mem_idx=0;mem_idx<mem_num;++mem_idx){

					auto key_nfa = reg(std::string("\"")+mem_key[mem_idx]+std::string("\""));
					auto pair_sep_nfa=reg(": ");
					auto val_nfa = reg(mem_val[mem_idx]);

					for(auto rho_idx:pair_sep_nfa.term)
						for(auto del_idx:pair_sep_nfa.rho[rho_idx].target_ref)
							pair_sep_nfa.del[del_idx].sem = { mem_beg_idx[mem_idx] };

					cog::nfa_tt<idx_t> term_nfa = [&]{
						if(mem_idx+1==mem_num)
							return lit('}');
						else
							return reg(", ");
					}();

					for(auto del_idx:term_nfa.rho[term_nfa.init].source_ref)
						term_nfa.del[del_idx].sem = { mem_end_idx[mem_idx] };

					nfa = cog::con(nfa,key_nfa,pair_sep_nfa,val_nfa,term_nfa);
				}
				return nfa;
			}();
/*
			{
				std::ofstream file("elem_nfa.dot");
				cog::gen::dot(file,elem_nfa);
			}
			system("dot -Tpng -oelem_nfa.png elem_nfa.dot");
*/
			auto file_nfa = [&]{
				auto file_beg_nfa=lit('[');

				for(auto rho_idx:file_beg_nfa.term)
					for(auto del_idx:file_beg_nfa.rho[rho_idx].target_ref)
						file_beg_nfa.del[del_idx].sem = { elem_beg_idx };

				auto elem_sep_nfa = reg(",");

				for(auto del_idx:elem_sep_nfa.rho[elem_sep_nfa.init].source_ref)
					elem_sep_nfa.del[del_idx].sem.insert(elem_end_idx);

				for(auto rho_idx:elem_sep_nfa.term)
					for(auto del_idx:elem_sep_nfa.rho[rho_idx].target_ref)
						elem_sep_nfa.del[del_idx].sem.insert(elem_beg_idx);

				auto file_end_nfa=lit(']');

				for(auto del_idx:file_end_nfa.rho[file_end_nfa.init].source_ref)
					file_end_nfa.del[del_idx].sem = { elem_end_idx };
		
				return cog::con(file_beg_nfa,elem_nfa,cog::rep(cog::con(elem_sep_nfa,elem_nfa)),file_end_nfa,cog::opt(lit('\n')));
			}();
			/*
			{
				std::ofstream file("file_nfa.dot");
				cog::gen::dot(file,file_nfa);
			}
			system("dot -Tpng -ofile_nfa.png file_nfa.dot");
			*/
			cog::dfa_tt<idx_t> dfa;
			std::map<idx_t,std::set<idx_t>> inv_min_sym;
			std::vector<idx_t> inv_min_al;

			std::tie(dfa,std::ignore) = cog::to_dfa(file_nfa);
//			std::tie(dfa,std::ignore) = cog::min_rho(dfa);
			std::tie(dfa,inv_min_sym) = cog::min_sym(dfa);
			std::tie(dfa,inv_min_al) = cog::min_al(dfa);
			/*
			{
				std::ofstream file("dfa.dot");
				cog::gen::dot(file,dfa);
			}
			system("dot -Tpng -odfa.png dfa.dot");
			*/
			ascii_map = cog::sym_map(cog::ascii::alpha<idx_t>,inv_min_sym,inv_min_al);
			rho_init=dfa.init;
			rho_undef=dfa.undef;
			std::tie(del_tab,term_tab) = cog::gen::tab(dfa);

			del_sem_tab = std::vector<std::vector<std::vector<idx_t>>>(dfa.rho.size(),std::vector<std::vector<idx_t>>(dfa.al.sym_num));
			for(idx_t del_idx=0;del_idx<dfa.del.size();++del_idx){
				const auto& del=dfa.del[del_idx];
				for(auto sem_idx:del.sem)
					del_sem_tab[del.source][del.sym].push_back(sem_idx);
			}

		}

		std::random_device rd;
		std::mt19937 mt(rd());
		char cmd;
		tio::mode_t mode;
		do{
			std::cin>>cmd;
			switch(cmd){
			case 'l':{
				std::cout<<"computing len...\n";
				auto start=chr::high_resolution_clock::now();
				auto len=std::strlen(data);
				auto stop=chr::high_resolution_clock::now();
				std::cout
					<<"done... interval = "<<chr::duration_cast<chr::milliseconds>(stop-start).count()<<"ms.\n"
					<<"length = "<<len<<"\n";
				break;
			}
			case 't':{
				char sum='\0';
				std::cout<<"benchmarking...\n";
				auto start=chr::high_resolution_clock::now();
				for(std::size_t idx=0;idx<size;++idx)
					sum+=*(data+idx);
				auto stop=chr::high_resolution_clock::now();
				std::cout
					<<"done... interval = "<<chr::duration_cast<chr::milliseconds>(stop-start).count()<<"ms.\n"
					<<"sum = "<<(int)(sum)<<"\n";
				break;
			}
			case 'c':{
				std::cout<<"parsing cog...\n";
				idx_t rho_idx=rho_init;
				std::size_t data_idx=0;
				idx_t sym_idx=0;
				auto start=chr::high_resolution_clock::now();
				for(
					sym_idx=ascii_map[data[data_idx]];
					data_idx<size&&sym_idx!=rho_undef&&rho_idx!=rho_undef;
					sym_idx=ascii_map[data[++data_idx]]
				){ 
					for(auto act_idx:del_sem_tab[rho_idx][sym_idx])
						if(act[act_idx])
							act[act_idx](data,data_idx);
					rho_idx=del_tab[rho_idx][sym_idx];
				}
				auto stop=chr::high_resolution_clock::now();
				if(rho_idx==rho_undef){
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
				else if(sym_idx==rho_undef)
					std::cout<<"fail, unknown input at "<<data_idx<<" = "<<(int)(data[data_idx])<<"\n";
				else if(data_idx!=size)
					std::cout<<"fail, premature exit at "<<data_idx<<" = "<<(int)(data[data_idx])<<"\n";
				else if(!term_tab[rho_idx])
					std::cout<<"fail, not term\n";
				else{
					std::cout
						<<"done\n"
						<<"\tinterval = "<<chr::duration_cast<chr::milliseconds>(stop-start).count()<<"ms\n"
						<<"\tnum_points = "<<tpos.size()<<"\n"
						<<"\tmax_id = "<<max_id_val<<"\n"
						<<"\tprox_filter_num = "<<prox_filter_num<<"\n"
						<<"\ttpos_filter_num = "<<tpos_filter_num<<"\n"
						<<"\tpos_filter_num = "<<pos_filter_num<<"\n";
				}
				break;
			}
			case 'w':{
				std::cout<<"writing to cdf...\n";
				cdf::file_t file = cdf::mk_file("ts.nc");
				cdf::dim_t trace_dim(file,"trace_dim",cdf::unlim);
				cdf::dim_t time_dim(file,"time_dim",cdf::unlim);
				cdf::var_tt<int16_t,2> 
					xpos_var(file,{&trace_dim,&time_dim},"xpos"),
					ypos_var(file,{&trace_dim,&time_dim},"ypos");
				cdf::var_tt<int64_t,2> 
					tpos_var(file,{&trace_dim,&time_dim},"tpos");
				cdf::var_tt<uint64_t,1>  num_var(file,{&trace_dim},"num");
				enddef(file);
				{
					std::vector<std::size_t> num_data;
					std::size_t skip_num=0;
					for(std::size_t i=0;i<tpos.size();++i){
						if(tpos[i].empty()){
							++skip_num;
							continue;
						}
						cdf::put(xpos_var,std::array<std::size_t,2>{i,0},std::array<std::size_t,2>{1,xpos[i].size()},xpos[i].data());
						cdf::put(ypos_var,std::array<std::size_t,2>{i,0},std::array<std::size_t,2>{1,ypos[i].size()},ypos[i].data());
						cdf::put(tpos_var,std::array<std::size_t,2>{i,0},std::array<std::size_t,2>{1,tpos[i].size()},tpos[i].data());
						std::size_t num=tpos[i].size();
						cdf::put(num_var,std::array<std::size_t,1>{i},std::array<std::size_t,1>{1},&num);
					}
					std::cout<<"skip num = "<<skip_num<<"\n";
				}
				std::cout<<"done...\n";
				break;
			}
			case 'u':{
				{
					std::uniform_int_distribution<std::size_t> dist(0,tpos.size()-1);
					std::size_t rand_idx=dist(mt);
					std::cout
						<<"random trace "<<rand_idx<<"\n"<<std::endl
						<<"\tnum: "<<tpos[rand_idx].size()
						<<"\tbeg: "<<cal.to_string(tpos[rand_idx].front())
						<<"\tend: "<<cal.to_string(tpos[rand_idx].back())
						<<"\n";
				}
				break;
			}
			case 'q':
				std::cout<<"closing...\n";
				munmap(data,size);
				break;
			}
		}while(cmd!='q');
	}catch(const std::exception& e){
		std::cerr<<"exception caught:\n"<<e.what()<<std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
