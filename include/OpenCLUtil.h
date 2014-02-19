#ifndef _OPENCLUTIL_H_
#define _OPENCLUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <CL/cl.h>

int createProgramFromSource(
	       	cl_device_id *_device, 
		cl_context *_context,
	       	const char *_program_file,
	       	cl_program *_program
);
int errorfunc(char *string, int err);
cl_kernel createKernel(const cl_program*,const char *);
void getBufferInfo(char *_name, cl_mem);
char *getCLErrorString(cl_int);
#ifdef __cplusplus
}
#endif

#endif
