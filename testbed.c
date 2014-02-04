#define _CRT_SECURE_NO_WARNINGS
#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

#define PROGRAM_FILE "ass.ptx"
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

void printtime(struct timespec spec) {

	printf(" %lu.%09lu ", spec.tv_sec, spec.tv_nsec);

}
/* Create program from a file and compile it */

#define MEM_SIZE (128)	
#define MAX_BINARY_SIZE (0x100000)	

cl_program build_program(cl_context ctx, cl_device_id dev, 
		const char* filename, char* options) {

	cl_int err;

	FILE *fp;
	size_t binary_size;
	char *binary_buf;
	cl_int binary_status;
	size_t log_size;
	char *program_log;
	/* Load kernel binary */
	fp = fopen(filename, "r");
	if (!fp) {
	}
	binary_buf = (char *)malloc(MAX_BINARY_SIZE);
	binary_size = fread(binary_buf, 1, MAX_BINARY_SIZE, fp);
	fclose(fp);

	cl_program program;
	program = clCreateProgramWithBinary(ctx, 1, &dev, (const size_t *)&binary_size,
			(const unsigned char **)&binary_buf, &binary_status, &err);

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

	if(argc != 4) {
		printf("Usage: ./program <filename> <samples> <batches>\n");
		exit(1);
	}
	char *file = argv[1];
	unsigned int samples = atoi(argv[2]);
	unsigned int nBatches = atoi(argv[3]);

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
	program = build_program(context, device, file, "-I ./");
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

	float *array = (float *)malloc(sizeof(float)*samples*3);
	float *results = (float *)malloc(sizeof(float)*samples);

	for(int i =0; i<samples*3; i++) {
		array[i] = rand() %5; 
	}

	sample_buf = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*samples*3, NULL, &err);
	if(err < 0) {
		perror("Couldn't create a memory object");
		exit(1);   
	};
	clEnqueueWriteBuffer(queue, sample_buf, CL_FALSE, 0, sizeof(float)*samples, array, 0, NULL, NULL);

	result_buf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 
			sizeof(float)*samples, NULL, &err);
	if(err < 0) {
		perror("Couldn't create a memory object");
		exit(1);   
	};
	int batchsize = samples/nBatches;

	struct timespec time1, time2;

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
	for(int i = 0; i<nBatches; i++) {

		cl_buffer_region region; 
		region.size=   batchsize*3*sizeof(float);
		region.origin= batchsize*3*sizeof(float)*i;
		sub_buf = clCreateSubBuffer( sample_buf, CL_MEM_READ_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &region, &err);
		if(err < 0) {
			printf("Couldn't create buffer 1, %d\n", err);
			exit(1);   
		};

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
	printf("\n");
	/* Enqueue kernel */

	/* Read and print the result */

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

