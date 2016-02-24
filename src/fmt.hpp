#pragma once
#include<cstdint>
#include<string>
#include<cdf/cdf.hpp>

namespace var{
	namespace type{ 
		using pos=int16_t;
		using tim=int64_t;
		using num=uint64_t;

	}
	using pos_t=cdf::var_tt<type::pos,2>;
	using tim_t=cdf::var_tt<type::tim,2>;
	using num_t=cdf::var_tt<type::num,1>;
	constexpr std::size_t num=4;
	namespace name{
		static const std::string
			xpos="xpos",
			ypos="ypos",
			tpos="tpos",
			num="num";
	}
}
namespace dim{
	constexpr std::size_t num=2;
	namespace name{
		static const std::string
			trace="trace_dim",
			time="time_dim";
	}
}
