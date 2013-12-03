#ifndef _OPENCLUTIL_H_
#define _OPENCLUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <CL/cl.h>

struct OpenCLHandle {
	cl_command_queue queue;
	cl_context context;
	cl_platform_id platform;
	cl_device_id device;
};

int initOpenCLContext(struct OpenCLHandle*);
int initOpenCLGLContext(struct OpenCLHandle*);
int createProgramFromSource(struct OpenCLHandle*, 
		const char *_program_file,
		cl_program *_program);
int errorfunc(char *string, int err);
cl_kernel createKernel(const cl_program*,const char *);
void getBufferInfo(char *_name, cl_mem);
char *getCLErrorString(cl_int);
#ifdef __cplusplus
}
#endif

#endif
