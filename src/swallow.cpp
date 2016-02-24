#include<cstdio>
#include<cstdint>
#include<cstdlib>
#include<cstring>
#include<iostream>
#include<fstream>
#include<stdexcept>
#include<string>

#include<cdf/cdf.hpp>
#include<tio/tio.hpp>
#include<xl/xl.hpp>
#include<fmt.hpp>

int
	main(int argc,char** argv)
{
	try{
		if(argc!=2)
			throw std::runtime_error("usage: swallow [netcdf_file]");

		std::string filename=argv[1];
		std::cout<<"swallowing "<<filename<<std::endl;
		cdf::file_t file=cdf::opn<>(filename);


		auto dim_num=cdf::dim_num(file);
		std::cout<<"file id = "<<file.id<<"\n";
		std::cout<<"dim_num: "<<dim_num<<std::endl;

		cdf::dim_t 
			trace_dim=cdf::lu_dim(file,dim::name::trace),
			time_dim=cdf::lu_dim(file,dim::name::time);
		std::cout
			<<"trace_dim: "<<trace_dim.len<<"\n"
			<<"time_dim: "<<time_dim.len<<"\n";

		var::num_t num=cdf::lu_var<var::type::num,1>(file,{&trace_dim},var::name::num);

		{
			std::vector<std::size_t> num_dat(trace_dim.len);
			cdf::get(num,{0},{trace_dim.len},num_dat.data());
			std::size_t zer_num=0;
			for(std::size_t i:num_dat)
				if(i==0)
					++zer_num;
			std::cout<<"zer_num: "<<zer_num<<"\n";
		}

		var::pos_t 
			xpos=cdf::lu_var<var::type::pos,2>(file,{&trace_dim,&time_dim},var::name::xpos),
			ypos=cdf::lu_var<var::type::pos,2>(file,{&trace_dim,&time_dim},var::name::ypos);

		xl::display_t display;
		xl::visual_info_t visual_info(display,32,TrueColor);
		xl::colormap_t colormap(display,visual_info);
		xl::window_t window(display,visual_info,colormap);
		xl::gc_tt<xl::window_t> gc(display,window);
		xl::source_tt<xl::event::group::structure> source(display,window);
		xl::map(window);

		xl::color_t 
			white(display,colormap,"#FFFFFF"),
			black(display,colormap,"#000000");

		xl::modify<xl::gc::foreground,xl::gc::background>(display,gc,xl::pixel(white),xl::pixel(black));

		std::size_t width,height;

		std::tie(width,height) = xl::measure<xl::window::width,xl::window::height>(window);

		constexpr double abs_max_x=1000.0;
		constexpr double abs_max_y=1500.0;
		short zoom=1;
		auto pos_tf = [&](	
			int16_t x,
			int16_t y
		){
			short x_=static_cast<short>(static_cast<double>(width)*(static_cast<double>(x)+abs_max_x)*zoom/(2*abs_max_x));
			short y_=static_cast<short>(static_cast<double>(height)*(static_cast<double>(y)+abs_max_y)*zoom/(2*abs_max_y));
			return xl::point_t{x_,y_};
		};
		std::string idx_str;
		std::size_t idx;
		std::size_t num_dat;
		std::vector<var::type::pos> 
			xpos_dat,
			ypos_dat;
		do{
			source(
				[&](auto& _event){
					if(std::is_same<std::decay_t<decltype(_event)>,xl::event::configure_t>::value){
						auto& event=(xl::event::configure_t&)(_event);
						width=event().width;
						height=event().height;
//						std::cout
//							<<"windows width = "<<width<<"\n"
//							<<"windows height = "<<height<<"\n";
					}
				}
			);
			std::getline(std::cin,idx_str);
			if(idx_str.empty())
				continue;

			if(idx_str=="m"){
				std::cout<<"zoom = "<<++zoom<<"\n";
			}else if(idx_str=="n"){
				std::cout<<"zoom = ";
				if(zoom>1)
					std::cout<<--zoom<<"\n";
				else
					std::cout<<zoom<<"\n";
			}else{
				idx=std::strtoull(idx_str.c_str(),nullptr,10);
				if(idx>=trace_dim.len){
					std::cout<<"index "<<idx<<"out of range: [0:"<<trace_dim.len<<")\n";
					continue;
				}
				cdf::get(num,{idx},{1},&num_dat);

				std::cout<<"num["<<idx<<"] = "<<num_dat<<"\n";

				xpos_dat.resize(num_dat);
				ypos_dat.resize(num_dat);
				std::cout<<"getting data...\n";
				cdf::get(xpos,{idx,0},{1,num_dat},xpos_dat.data());
				cdf::get(ypos,{idx,0},{1,num_dat},ypos_dat.data());
				std::cout<<"done!\n";
			}
			xl::clear(window);
			std::cout<<"points:\n";
			for(std::size_t i=0;i<num_dat;++i){
				xl::point_t point=pos_tf(xpos_dat[i],ypos_dat[i]);
				std::cout<<xpos_dat[i]<<", "<<ypos_dat[i]<<"\n";
				xl::draw(display,window,gc,point);
			}
			xl::flush(display);

		}while(!idx_str.empty());

	}catch(const std::exception& e){
		std::cerr<<"error: "<<e.what()<<std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;

}
