#pragma once
#include<cinttypes>
#include<iostream>
#include<vector>

namespace proto{

	constexpr uint16_t del_pos=1;
	using num_t=uint64_t;
	using pos_idx_t=uint16_t;
	using hier_idx_t=uint8_t;
	using pos_utc_t=int16_t; 
	using hier_utc_t=int32_t; //most time_delta < 300 secs, but hier can be longer... consider using 10 second resolution?

	struct point_t{
		pos_idx_t x,y;
		pos_utc_t t;
	};
	
	std::ostream& operator<<(std::ostream& o,const point_t& p){
		return o<<x<<y<<t;
	}

	std::istream& operator>>(std::istream& o,point_t& p){
		return o>>p.x>>p.y>>p.t;
	}

	struct hier_t{
		hier_idx_t b,f;
		hier_utc_t t;
	};

	std::ostream& operator<<(std::ostream& o,const hier_t& h){
		return o<<b<<f<<t;
	}
	std::istream& operator>>(std::istream& o,hier_t& h){
		return o>>b>>f>>t;
	}
		
	struct data_t{
		std::vector<std::vector<point_t>> P;
		std::vector<std::vector<hier_t>> H;
	};

	std::ostream& operator<<(std::ostream& o,const data_t& d){
		num_t N=P.size();
		o<<N;
		for(num_t idx0=0;idx0<N;++idx0){
			auto& p=d.P[idx0];
			num_t NP=p.size();
			o<<NP;
			for(num_t idx1=0;idx1<Np;++idx1)
				o<<p[idx1];
			auto& h=d.H[idx0]
			num_t NH=h.size();
			o<<NH;
			for(num_t idx1=0;idx1<Nh;++idx1)
				o<<h[idx1];
		}
		return o;
	}

	std::istream& operator>>(std::istream& o,data_t& d){
		num_t N;
		o>>N;
		d.P.resize(N);
		for(num_t idx0=0;idx0<N;++idx0){
			auto& p=d.P[idx0];
			o>>NP;
			num_t NP=p.resize(NP);
			for(num_t idx1=0;idx1<Np;++idx1)
				o>>p[idx1];
			auto& h=d.H[idx0]
			num_t NH;
			o>>NH;
			h.resize(NH);
			for(num_t idx1=0;idx1<Nh;++idx1)
				o>>h[idx1];
		}
		return o;
	}

}
