#include<chrono>
#include<cstdlib>
#include<iostream>
#include<exception>
#include<fstream>
#include<string>

#include<sys/mman.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>

#include<tio/tio.hpp>

using namespace std::literals;
namespace chr=std::chrono;

int
	main(int argc,char** argv)
{
	try{
		if(argc!=3)
			throw std::runtime_error("pin [memname] [filename]");
		std::string memname=argv[1],filename=argv[2];
	
		std::cout<<"pinning "<<filename<<" to "<<memname<<"\n";	

		int fd=shm_open(memname.c_str(),O_CREAT|O_TRUNC|O_RDWR,S_IRUSR|S_IWUSR);
		if(fd<0)
			throw std::runtime_error("shm_open fail");

		char* data=nullptr;
		std::size_t size;
		try{
		{
			std::ifstream file(filename);
			if(!file.is_open())
				throw std::runtime_error("ifstream ctor fail");
			std::cout<<"computing filesize... \n";
			file.seekg(0,std::ios::end);
			size=file.tellg();
			std::cout<<"done, size = "<<size<<"b.\n";
			std::cout<<"allocating memory...\n";
			if(ftruncate(fd,size)<0)
				throw std::runtime_error("allocation error\n");
			std::cout<<"done...\n";
			file.seekg(0,std::ios::beg);

			std::cout<<"mapping shared memory...\n";
			data=(char*)(mmap(nullptr,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0));
			if(data==MAP_FAILED)
				throw std::runtime_error("mmap failed\n");
			std::cout<<"done...\n";
			std::cout<<"copying file...\n";
			{
				auto start=chr::high_resolution_clock::now();
				std::copy((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>()),data);
				auto stop=chr::high_resolution_clock::now();
				std::cout<<"done, took: "<<chr::duration_cast<chr::milliseconds>(stop-start).count()<<" ms.\n";
			}
		}
		}catch(const std::exception& e){
			std::cerr<<e.what()<<std::endl;
		}
//		std::cout<<"mapping memory\n";

		tio::mode_t mode{};
		char input;
		std::size_t index=0;
		do{
			std::cin>>input;
			switch(input){
			case 'q':
				std::cout<<"unlink file\n";
				munmap(data,size);
				if(shm_unlink(memname.c_str())<0)
					throw std::runtime_error("sh_unlink fail");
				break;
			case 'r':
				if(index>=size){
					index=0;
					std::cout<<"\n";
				}else
					std::cout<<data[index++];
				break;
			}
		}while(input!='q');
	}catch(const std::exception& e){
		std::cerr<<e.what()<<std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
