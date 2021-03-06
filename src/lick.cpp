#include<algorithm>
#include<cstdlib>
#include<iostream>
#include<fstream>
#include<string>
#include<stdexcept>

#include<stdlib.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>

#include<lick.hpp>
#include<mouth.hpp>
#include<tongue/building_map.hpp>

#include<cdf/cdf.hpp>

int
	main(int argc,char** argv)
{
	try{
		if(argc<2)
			throw std::runtime_error("usage: lick [shm_in] [...]");

	
		mouth_t mouth(argc,argv);
		tongue::building_map_t tongue(argc,argv);
		mouth(tongue);

	}catch(const std::exception& e){
		std::cerr<<"error: "<<e.what()<<std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
