#ifndef _RESOURCES_H_
#define _RESOURCES_H_

#include <CL/cl.h>
#include <map>
#include <string>

class CL_Resources {
	public:
		CL_Resources();	
		~CL_Resources();
		void addKernel(cl_program*, const std::string &_name);
		void addMemObj(cl_mem*, const std::string &_name);
		cl_kernel *getKernel(const std::string&) const;
		cl_mem *getMemObj(const std::string&) const;
		void releaseMemory(const std::string&);
		void release();
	private:
		std::map<std::string, cl_mem*> m_memory;
		std::map<std::string, cl_kernel*> m_kernels;
		bool isReleased;
};

#endif 
