#include "resources.hpp"
#include "OpenCLUtil.h"
#include <iostream>
#include <cstdlib>


CL_Resources::CL_Resources():
	isReleased(false)
{}

CL_Resources::~CL_Resources() {

	if(isReleased != true) {
		release();
	}
}


void CL_Resources::release() {

	if(isReleased) return;
	std::cout<<"Releasing resources\n";
	std::map<std::string ,cl_mem*>::iterator memory_it;
	for(memory_it = m_memory.begin(); memory_it != m_memory.end(); memory_it++) {
		std::cout<<"Release memory: "<<memory_it->first<<'\n';
		clReleaseMemObject(*memory_it->second);
		if(memory_it->second != NULL)
			delete(memory_it->second);
	}
	std::map<std::string, cl_kernel*>::iterator kernel_it;
	for(kernel_it = m_kernels.begin(); kernel_it != m_kernels.end(); kernel_it++) {
		std::cout<<"Release kernel: "<<kernel_it->first<<'\n';
		clReleaseKernel(*kernel_it->second);
		if(kernel_it->second != NULL)
			delete(kernel_it->second);
	}
	isReleased = true;
}

void CL_Resources::addKernel(cl_program *_program, const std::string &_name) {

	cl_kernel *temp =  new cl_kernel(createKernel(_program, _name.c_str()));
	m_kernels.insert(std::pair<std::string, cl_kernel*>(_name, temp));
}

void CL_Resources::addMemObj(cl_mem *_obj, const std::string &_name) {

	m_memory.insert(std::pair<std::string, cl_mem*>(_name, _obj));
}

cl_kernel *CL_Resources::getKernel(const std::string &_name)const  {

	return m_kernels.find(_name)->second;

}

cl_mem *CL_Resources::getMemObj(const std::string &_name)const {
	return m_memory.find(_name)->second;
}

void CL_Resources::releaseMemory(const std::string &_name) {

	std::map<std::string ,cl_mem*>::iterator memory_it = m_memory.find(_name);
	if(memory_it == m_memory.end())
	       	std::cout << "couldn't find memory" <<'\n';
	else {
	       	clReleaseMemObject(*memory_it->second);
		m_memory.erase(memory_it);
	       	std::cout << "memory released" <<'\n';
	}
}
