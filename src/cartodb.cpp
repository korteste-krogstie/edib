#include<cstdlib>
#include<iostream>
#include<stdexcept>
#include<lick.hpp>
#include<mouth.hpp>
#include<tongue/cartodb.hpp>

int
	main(int argc,char** argv)
{
	try{
		if(argc!=3)
			throw std::runtime_error("usage: lick [shm_in] [file_out]");
	
		mouth_t mouth(argc,argv);
		tongue::cartodb_t tongue(argc,argv);
		mouth(tongue);

	}catch(const std::exception& e){
		std::cerr<<"error: "<<e.what()<<std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
