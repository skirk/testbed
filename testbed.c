#define _CRT_SECURE_NO_WARNINGS
#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

#define PROGRAM_FILE "ray_march.cl"
#define KERNEL_FUNC "ray_intervals"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/sysinfo.h>
#include <time.h>

#ifdef MAC
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

/* Find a GPU or CPU associated with the first available platform */
cl_device_id create_device() {

   cl_platform_id platform;
   cl_device_id dev;
   int err;

   /* Identify a platform */
   err = clGetPlatformIDs(1, &platform, NULL);
   if(err < 0) {
      perror("Couldn't identify a platform");
      exit(1);
   } 

   /* Access a device */
   err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
   if(err == CL_DEVICE_NOT_FOUND) {
      err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
   }
   if(err < 0) {
      perror("Couldn't access any devices");
      exit(1);   
   }

   return dev;
}
//timer function
struct timespec diff(struct timespec start, struct timespec end)
{
	struct timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

struct timespec add(struct timespec lhs, struct timespec rhs) {

	struct  timespec  result ;

	/* Add the two times together. */

	result.tv_sec = lhs.tv_sec + rhs.tv_sec ;
	result.tv_nsec = lhs.tv_nsec + rhs.tv_nsec ;
	if (result.tv_nsec >= 1000000000L) {		/* Carry? */
		result.tv_sec++ ;  result.tv_nsec = result.tv_nsec - 1000000000L ;
	}
	return (result) ;
}

void printtime(struct timespec spec) {

	printf(" %lu.%09lu ", spec.tv_sec, spec.tv_nsec);

}
/* Create program from a file and compile it */
cl_program build_program(cl_context ctx, cl_device_id dev, 
		const char* filename, char* options) {

	cl_program program;
	FILE *program_handle;
	char *program_buffer, *program_log;
	size_t program_size, log_size;
	int err;

	/* Read program file and place content into buffer */
	program_handle = fopen(filename, "r");
	if(program_handle == NULL) {
		perror("Couldn't find the program file");
		exit(1);
	}
	fseek(program_handle, 0, SEEK_END);
	program_size = ftell(program_handle);
	rewind(program_handle);
	program_buffer = (char*)malloc(program_size + 1);
	program_buffer[program_size] = '\0';
	fread(program_buffer, sizeof(char), program_size, program_handle);
	fclose(program_handle);

	/* Create program from file */
	program = clCreateProgramWithSource(ctx, 1, 
			(const char**)&program_buffer, &program_size, &err);
	if(err < 0) {
		perror("Couldn't create the program");
		exit(1);
	}
	free(program_buffer);

	/* Build program */
	struct timespec time1, time2;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
	err = clBuildProgram(program, 0, NULL, options, NULL, NULL);
	if(err < 0) {

		/* Find size of log and print to std output */
		clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
				0, NULL, &log_size);
		program_log = (char*) malloc(log_size + 1);
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
				log_size + 1, program_log, NULL);
		printf("%s\n", program_log);
		free(program_log);
		exit(1);
	}
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
	//printf("compilation ");
	//printtime(diff(time1, time2));

	return program;
}

int main(int argc, char* argv[]) {

	if(argc != 3) {
		printf("Usage: ./program <samples> <batches>\n");
		exit(1);
	}
	unsigned int samples = atoi(argv[1]);
	unsigned int nBatches = atoi(argv[2]);

	//force compilation = no caching
	//setenv("CUDA_CACHE_DISABLE", "1", 1);

	/* OpenCL data structures */
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;;
	cl_kernel kernel;
	cl_int err;

	/* Data and buffers */
	cl_mem sample_buf, result_buf, sub_buf, sub_buf2;

	/* Extension data */


	/* Create a device and context */
	device = create_device();
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	if(err < 0) {
		perror("Couldn't create a context");
		exit(1);   
	}

	/* Obtain the device data */


	/* Build the program and create the kernel */
	program = build_program(context, device, PROGRAM_FILE, "-I ./");
	kernel = clCreateKernel(program, KERNEL_FUNC, &err);
	if(err < 0) {
		perror("Couldn't create a kernel");
		exit(1);   
	};

	/* Create CL buffers to hold input and output data */

	queue = clCreateCommandQueue(context, device, 0, &err);
	if(err < 0) {
		perror("Couldn't create a command queue");
		exit(1);   
	};
	/* Create kernel arguments */

	//3 floats describe one point

	float   array[samples*3];
	float results[samples];

	for(int i =0; i<samples*3; i++) {
		array[i] = rand() %5; 
	}

	sample_buf = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(array), NULL, &err);
	if(err < 0) {
		perror("Couldn't create a memory object");
		exit(1);   
	};
	clEnqueueWriteBuffer(queue, sample_buf, CL_FALSE, 0, sizeof(array), array, 0, NULL, NULL);

	result_buf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 
			sizeof(results), NULL, &err);
	if(err < 0) {
		perror("Couldn't create a memory object");
		exit(1);   
	};
	int batchsize = samples/nBatches;

	struct timespec time1, time2;
	//struct timespec buffer1;
	//buffer1.tv_sec = 0;
	//buffer1.tv_nsec = 0;

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
	for(int i = 0; i<nBatches; i++) {

		cl_buffer_region region; 
		region.size=   batchsize*3*sizeof(float);
		region.origin= batchsize*3*sizeof(float)*i;
		//struct timespec buffer2, buffer3;

		//clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &buffer2);
		sub_buf = clCreateSubBuffer( sample_buf, CL_MEM_READ_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &region, &err);
		if(err < 0) {
			printf("Couldn't create buffer 1, %d\n", err);
			exit(1);   
		};
		//clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &buffer3);
		//buffer1 = add(buffer1, diff(buffer2, buffer3));

		cl_buffer_region region2; 
		region2.size=   batchsize*sizeof(float);
		region2.origin= batchsize*sizeof(float)*i;
		//printf("%zu\n", region2.origin);
		//printf("%zu\n", region2.size);
		sub_buf2 = clCreateSubBuffer(result_buf, CL_MEM_WRITE_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &region2, &err);
		if(err < 0) {
			printf("Couldn't create buffer 2, %d\n", err);
			exit(1);   
		};

		err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &sub_buf);
		if(err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);   
		};
		err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &sub_buf2);
		if(err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);   
		};
		size_t worksize = batchsize;
		err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &worksize, NULL, 0, NULL, NULL);
		if(err < 0) {
			perror("Couldn't enqueue the kernel");
			exit(1);   
		}
		clReleaseMemObject(sub_buf);
		clReleaseMemObject(sub_buf2);
	}
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
	printtime(diff(time1, time2));
	//printf("buffer time ");
	//printtime(buffer2);
	/* Enqueue kernel */


	err = clEnqueueReadBuffer(queue, result_buf, CL_TRUE, 0, 
			sizeof(results), &results, 0, NULL, NULL);
	if(err < 0) {
		perror("Couldn't read the output buffer");
		exit(1);   
	}
	//printf("The kernel result is %f\n", result);   

	/* Deallocate resources */
	clReleaseMemObject(sample_buf);
	clReleaseMemObject(result_buf);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);
	return 0;
}

