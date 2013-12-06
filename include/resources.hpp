#ifndef _RESOURCES_H_
#define _RESOURCES_H_

#include <CL/cl.h>
#include <map>
#include <string>

class CL_Resources {
	public:
		cl_command_queue queue;
		cl_context context;
		cl_platform_id platform;
		cl_device_id device;

		void initOpenCLContext();
		void initOpenCLGLContext();

		static CL_Resources &getInstance();

		void addKernel(cl_program*, const std::string &_name);
		void addMemObj(cl_mem*, const std::string &_name);
		cl_kernel *getKernel(const std::string&) const;
		cl_mem *getMemObj(const std::string&) const;

		void releaseMemory(const std::string&);
		void release();
	private:
		CL_Resources();	
		~CL_Resources();
		CL_Resources(const CL_Resources&);
		static CL_Resources *m_instance;
		std::map<std::string, cl_mem*> m_memory;
		std::map<std::string, cl_kernel*> m_kernels;
		bool isReleased;
};

#endif 
