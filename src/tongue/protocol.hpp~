#pragma once
#include<map>
#include<vector>
#include<fstream>
#include<functional>
#include<stdexcept>
#include<string>

#include<lick.hpp>
#include<protocol.hpp>

#include<utc.hpp>
namespace tongue{

	struct protocol_t{
		std::vector<std::pair<utc::time_t,uint32_t>> trace_tab;
		std::map<std::reference_wrapper<std::pair<utc::time_t,uint32_t>>,std::size_t> trace_map;

		std::vector<std::pair<uint8_t

		std::vector<std::string> building_tab,floor_tab;
		std::map<std::reference_wrapper<std::string>,std::size_t> building_map,floor_map;

		std::size_t 
			aux_idx,aux_num,
			floor_idx,
			building_idx,
			trace_idx;
		std::ofstream file;

		protocol_t(int argc,char** argv){
			if(argc!=3)
				throw std::runtime_error("too few args... arg[2] = outfile");
			file=std::ofstream(argv[2]);
			if(!file.is_open())
				throw std::runtime_error("couldn't open outfile...\n");
		}

		~protocol_t(){

		}

		void beg(char*,std::size_t){}
		void end(char*,std::size_t){
			std::vector<uint64_t> idx_tab(building_tab.size());
			{
				uint64_t idx=0;
				std::generate(idx_tab.begin(),idx_tab.end(),[&]{ return idx++; });
			}
			std::sort(idx_tab.begin(),idx_tab.end(),[&](auto i0,auto i1){ return building_count_tab[i0]>=building_count_tab[i1]; });
			for(auto i:idx_tab)
				file<<building_tab[i]<<": "<<building_count_tab[i]<<"\n";
		}

		void beg(tag::hierarchy,char*,std::size_t){}
		void end(tag::hierarchy,char* data,std::size_t idx){
			std::size_t 
				building_idx=building_tab.size();

			auto building_it=building_map.insert({std::string(data+build_str_idx,build_str_num),building_idx});
			if(!building_it.second)
				building_idx=building_it.first->second;
			else{
				building_tab.push_back(building_it.first->first);
				building_count_tab.push_back(0);
			}

			++building_count_tab[building_idx];
		}

		void beg(tag::building,char*,std::size_t idx){
			aux_idx=idx;
		}
		void end(tag::building,char* data,std::size_t idx){
			aux_num=idx-aux_idx;
			std::string s(data+aux_idx,aux_num);
			std::size_t i=building_tab.size();
			auto it=building_map.insert({s,i});
			if(!it.second)
				i=it.first->second;
			else
				building_tab.push-back(std::move(s));
		}

		void beg(tag::floor,char*,std::size_t idx){
			
		}
		void end(tag::floor,char*,std::size_t idx){

		}

		void beg(tag::latitude,char*,std::size_t){

		}
		void end(tag::longitude,char*,std::size_t){

		}

		void beg(tag::timestamp,char*,std::size_t){

		}
		void end(tag::timestamp,char*,std::size_t){

		}
		void beg(tag::id,char*,std::size_t){

		}
		void end(tag::id,char*,std::size_t){

		}

		template<class tag>
		void beg(tag,char*,std::size_t){}

		template<class tag>
		void end(tag,char*,std::size_t){}


	};

}
