#include "OpenCLUtil.h"

#include <stdio.h>

int errorfunc(char *string, int err) {
	printf("error code %d\n", err);
	perror(string);
	return 1;
}

int createProgramFromSource(cl_device_id *_device, cl_context *_context, const char *_program_file, cl_program *_program) {
	cl_int err;
	FILE *program_handle;
	char *program_buffer, *program_log;
	size_t program_size, log_size;
	/* Read program file and place content into buffer */
	program_handle = fopen(_program_file, "r");

	if(program_handle == NULL) return errorfunc("Couldn't find the program file", 1);
	fseek(program_handle, 0, SEEK_END);
	program_size = ftell(program_handle);
	rewind(program_handle);
	program_buffer = (char*)malloc(program_size + 1);
	program_buffer[program_size] = '\0';
	fread(program_buffer, sizeof(char), program_size, program_handle);
	fclose(program_handle);

	/* Create program from file */
	*_program = clCreateProgramWithSource(*_context, 1, (const char**)&program_buffer, &program_size, &err);
	if(err < 0) return errorfunc("Couldn't create the program", err);
	free(program_buffer);

	/* Build program */
	err = clBuildProgram(*_program, 0, NULL, NULL, NULL, NULL);
	if(err < 0) {

		/* Find size of log and print to std output */
		clGetProgramBuildInfo(*_program, *_device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		program_log = (char*) malloc(log_size + 1);
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(*_program, *_device, CL_PROGRAM_BUILD_LOG, log_size + 1, program_log, NULL);
		printf("%s\n", program_log);
		free(program_log);
		errorfunc("Something went wrong", 1);
	}
	return 0;
}

void getBufferInfo(char *_name, cl_mem _mem) {
	size_t buffer_size;
	void *buffer_mem;
	clGetMemObjectInfo(_mem, CL_MEM_SIZE, sizeof(buffer_size), &buffer_size, NULL); 
	clGetMemObjectInfo(_mem, CL_MEM_HOST_PTR, sizeof(buffer_mem), &buffer_mem, NULL); 
	printf("Buffer name: %s\n", _name);
	printf("Buffer size: %lu\n", buffer_size);
	printf("Buffer address: %p\n", buffer_mem);

}

cl_kernel createKernel(const cl_program *_program, const char *_kernel) {

	cl_int err;
	printf("kernel name: %s\n", _kernel);
	cl_kernel kernel = clCreateKernel(*_program, _kernel, &err);
	if(err < 0) printf("Couldn't create the kernel %s\n", getCLErrorString(err));
	return kernel;
}


char *getCLErrorString(cl_int err) {
	switch (err) {
		case CL_SUCCESS: return (char *) "Success!";
		case CL_DEVICE_NOT_FOUND: return (char *) "Device not found.";
		case CL_DEVICE_NOT_AVAILABLE: return (char *) "Device not available";
		case CL_COMPILER_NOT_AVAILABLE: return (char *) "Compiler not available";
		case CL_MEM_OBJECT_ALLOCATION_FAILURE: return (char *) "Memory object allocation failure";
		case CL_OUT_OF_RESOURCES: return (char *) "Out of resources";
		case CL_OUT_OF_HOST_MEMORY: return (char *) "Out of host memory";
		case CL_PROFILING_INFO_NOT_AVAILABLE: return (char *) "Profiling information not available";
		case CL_MEM_COPY_OVERLAP: return (char *) "Memory copy overlap";
		case CL_IMAGE_FORMAT_MISMATCH: return (char *) "Image format mismatch";
		case CL_IMAGE_FORMAT_NOT_SUPPORTED: return (char *) "Image format not supported";
		case CL_BUILD_PROGRAM_FAILURE: return (char *) "Program build failure";
		case CL_MAP_FAILURE: return (char *) "Map failure";
		case CL_INVALID_VALUE: return (char *) "Invalid value";
		case CL_INVALID_DEVICE_TYPE: return (char *) "Invalid device type";
		case CL_INVALID_PLATFORM: return (char *) "Invalid platform";
		case CL_INVALID_DEVICE: return (char *) "Invalid device";
		case CL_INVALID_CONTEXT: return (char *) "Invalid context";
		case CL_INVALID_QUEUE_PROPERTIES: return (char *) "Invalid queue properties";
		case CL_INVALID_COMMAND_QUEUE: return (char *) "Invalid command queue";
		case CL_INVALID_HOST_PTR: return (char *) "Invalid host pointer";
		case CL_INVALID_MEM_OBJECT: return (char *) "Invalid memory object";
		case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: return (char *) "Invalid image format descriptor";
		case CL_INVALID_IMAGE_SIZE: return (char *) "Invalid image size";
		case CL_INVALID_SAMPLER: return (char *) "Invalid sampler";
		case CL_INVALID_BINARY: return (char *) "Invalid binary";
		case CL_INVALID_BUILD_OPTIONS: return (char *) "Invalid build options";
		case CL_INVALID_PROGRAM: return (char *) "Invalid program";
		case CL_INVALID_PROGRAM_EXECUTABLE: return (char *) "Invalid program executable";
		case CL_INVALID_KERNEL_NAME: return (char *) "Invalid kernel name";
		case CL_INVALID_KERNEL_DEFINITION: return (char *) "Invalid kernel definition";
		case CL_INVALID_KERNEL: return (char *) "Invalid kernel";
		case CL_INVALID_ARG_INDEX: return (char *) "Invalid argument index";
		case CL_INVALID_ARG_VALUE: return (char *) "Invalid argument value";
		case CL_INVALID_ARG_SIZE: return (char *) "Invalid argument size";
		case CL_INVALID_KERNEL_ARGS: return (char *) "Invalid kernel arguments";
		case CL_INVALID_WORK_DIMENSION: return (char *) "Invalid work dimension";
		case CL_INVALID_WORK_GROUP_SIZE: return (char *) "Invalid work group size";
		case CL_INVALID_WORK_ITEM_SIZE: return (char *) "Invalid work item size";
		case CL_INVALID_GLOBAL_OFFSET: return (char *) "Invalid global offset";
		case CL_INVALID_EVENT_WAIT_LIST: return (char *) "Invalid event wait list";
		case CL_INVALID_EVENT: return (char *) "Invalid event";
		case CL_INVALID_OPERATION: return (char *) "Invalid operation";
		case CL_INVALID_GL_OBJECT: return (char *) "Invalid OpenGL object";
		case CL_INVALID_BUFFER_SIZE: return (char *) "Invalid buffer size";
		case CL_INVALID_MIP_LEVEL: return (char *) "Invalid mip-map level";
		default: return (char *) "Unknown";
	}
}		   
