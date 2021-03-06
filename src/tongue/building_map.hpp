#pragma once
#include<algorithm>
#include<map>
#include<fstream>
#include<set>
#include<stdexcept>
#include<string>
#include<utility>
#include<vector>
#include<lick.hpp>
namespace tongue{

	struct building_map_t{
		std::map<std::string,std::size_t> building_map;
		std::vector<std::string> building_tab;
		std::vector<std::size_t> building_count_tab;
		std::size_t build_str_idx,build_str_num;
		std::ofstream file;

		building_map_t(int argc,char** argv){
			if(argc!=3)
				throw std::runtime_error("too few args... arg[2] = outfile");
			file=std::ofstream(argv[2]);
			if(!file.is_open())
				throw std::runtime_error("couldn't open outfile...\n");
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
			build_str_idx=idx;
		}
		void end(tag::building,char* data,std::size_t idx){
			build_str_num=idx-build_str_idx;
		}

		template<class tag>
		void beg(tag,char*,std::size_t){}

		template<class tag>
		void end(tag,char*,std::size_t){}


		~building_map_t(){

		}
	};

}
