#pragma once
#pragma once
#include<algorithm>
#include<cstring>
#include<cmath>
#include<iomanip>
#include<iostream>
#include<fstream>
#include<stdexcept>
#include<string>
#include<utility>
#include<vector>
#include<lick.hpp>
#include<utc.hpp>

/*
{
    "type":"FeatureCollection",
    "features":[
        {
            "type":"Feature",
            "geometry":
                {
                    "type": "Point",
                    "coordinates":[10.40559,63.41522]
                }
        },
        {
            "type":"Feature",
            "geometry":
                {
                    "type": "Point",
                    "coordinates":[10.40671,63.41558]
                }
        }
    ]
}
*/

namespace tongue{

	constexpr std::size_t sig_dig=4;
	constexpr std::size_t 
		file_lim=1000000,
		file_thresh=100
	;
	struct cartodb_t{
		std::ofstream file;
		std::size_t aux_idx,aux_num;

		std::vector<double> lon,lat;
		std::vector<utc::time_t> time;

		const float aux_pow = std::pow(10.0f,sig_dig);

		cartodb_t(int argc,char** argv){
			file=std::ofstream(argv[2]);
			if(!file.is_open())
				throw std::runtime_error(std::string("couldn't open outfile")+argv[2]);
			file<<std::fixed<<std::setprecision(sig_dig);
		}

		void beg(char*,std::size_t){}
		void end(char*,std::size_t){
			file
				<<"{"
				<<"\"type\":\"FeatureCollection\","
				<<"\"features\":[";

			bool done=false;
			for(std::size_t idx=0;idx<lon.size()&&!done;++idx){
				file
					<<"{"
					<<"\"type\":\"Feature\","
					<<"\"geometry\":"
					<<"{"
					<<"\"type\":\"Point\","
					<<"\"coordinates\":["<<lon[idx]<<","<<lat[idx]<<"]"
					<<"}"
					<<"}";
				file.flush();
				if((std::size_t)(file.tellp())+file_thresh>=file_lim)
					done=true;
				else
					file<<",";
			}

			file
				<<"]"
				<<"}";
		}

		void beg(tag::element,char* data,std::size_t idx){}
		void end(tag::element,char* data,std::size_t idx){}

		void beg(tag::timestamp,char* data,std::size_t idx){
			aux_idx=idx;
		}
		void end(tag::timestamp,char* data,std::size_t idx){
			char backup=data[idx];
			data[idx]='\0';
			time.push_back(std::strtoll(data+aux_idx,nullptr,10));
			data[idx]=backup;
		}

		void beg(tag::longitude,char* data,std::size_t idx){
			aux_idx=idx;
		}
		void end(tag::longitude,char* data,std::size_t idx){
			char backup=data[idx];
			data[idx]='\0';
			lon.emplace_back();
			lon.back()=std::trunc(std::strtof(data+aux_idx,nullptr)*aux_pow)/aux_pow;
			data[idx]=backup;
		}

		void beg(tag::latitude,char* data,std::size_t idx){
			aux_idx=idx;
		}
		void end(tag::latitude,char* data,std::size_t idx){
			char backup=data[idx];
			data[idx]='\0';
			lat.emplace_back();
			lat.back()=std::trunc(std::strtof(data+aux_idx,nullptr)*aux_pow)/aux_pow;
			data[idx]=backup;
		}

		template<class tag>
		void beg(tag,char*,std::size_t){}

		template<class tag>
		void end(tag,char*,std::size_t){}

	};

}

