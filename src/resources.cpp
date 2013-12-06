#include <CL/cl_gl.h>
#include <GL/glx.h>
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include "OpenCLUtil.h"
#include "error.hpp"

CL_Resources* CL_Resources::m_instance = NULL;


CL_Resources &CL_Resources::getInstance()
{
	if(m_instance == NULL) {
		m_instance = new CL_Resources();
	}
	return *m_instance;
}

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

void CL_CALLBACK contexterror(const char *errinfo, const void *private_info, size_t cb, void *user_data) {
	printf("context error %s\n", errinfo);
}

void CL_Resources::initOpenCLContext() {

	cl_int err;

	err = clGetPlatformIDs(1, &platform, NULL);
	if(err < 0) interrupt("Couldn't find any platforms", err);

	/* Access a device */
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	if(err < 0)  interrupt("Couldn't find any devices", err);

	char buf_data[1028];
	cl_ulong max_alloc;
	clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(buf_data), &max_alloc, NULL);
	printf("max allocation size: %lu\n", (unsigned long)max_alloc);

	char buf_data2[1028];
	cl_ulong max_alloc1;
	clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(buf_data2), &max_alloc1, NULL);
	printf("max memory size: %lu\n", (unsigned long)max_alloc1);

	context = clCreateContext(NULL, 1, &device, &contexterror, NULL, &err);
	if(err < 0) interrupt("Couldn't create a context", err);


}

void CL_Resources::initOpenCLGLContext() {
	cl_int err;

	err = clGetPlatformIDs(1, &platform, NULL);
	if(err < 0)  interrupt("Couldn't find any platforms", err);

	/* Access a device */
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	if(err < 0) interrupt("Couldn't find any devices", err);

	char buf_data[1028];
	cl_ulong max_alloc;
	clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(buf_data), &max_alloc, NULL);
	printf("max allocation size: %lu\n", (unsigned long)max_alloc);

	char buf_data2[1028];
	cl_ulong max_alloc1;
	clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(buf_data2), &max_alloc1, NULL);
	printf("max memory size: %lu\n", (unsigned long)max_alloc1);

	cl_context_properties properties[] = {
		CL_GL_CONTEXT_KHR, (cl_context_properties) glXGetCurrentContext(),
		CL_GLX_DISPLAY_KHR, (cl_context_properties) glXGetCurrentDisplay(),
		CL_CONTEXT_PLATFORM, (cl_context_properties) platform, 
		0};


	context = clCreateContext(properties, 1, &device, NULL, NULL, &err);
	if(err < 0) interrupt("Couldn't create a context", err);

	queue = clCreateCommandQueue(context, device, 0, &err);
	if(err < 0) {
		perror("Couldn't create a command queue");
		exit(1);
	};

}


