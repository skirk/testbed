
#define PROGRAM_FILE "ray_march.cl"
#define KERNEL_FUNC "evaluate_intervals"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include <vector>
#include <sys/sysinfo.h>
#include <time.h>
#include <math.h>

#ifdef MAC
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

cl_device_id device;
cl_context context;
cl_command_queue queue;
cl_program program;;
cl_kernel kernel;
cl_int err;
float EPSILON = 0.02;

cl_mem ray_buf, interval_buf;

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
cl_float3 operator*(const cl_float3 lhs, float rhs) {
	cl_float3 temp;
	temp.s[0] = lhs.s[0]*rhs;
	temp.s[1] = lhs.s[1]*rhs;
	temp.s[2] = lhs.s[2]*rhs;
	return temp;
}
cl_float3 operator/(const cl_float3 lhs, float rhs) {
	cl_float3 temp;
	temp.s[0] = lhs.s[0]/rhs;
	temp.s[1] = lhs.s[1]/rhs;
	temp.s[2] = lhs.s[2]/rhs;
	return temp;
}
cl_float3 operator+(const cl_float3 lhs, cl_float3 rhs) {
	cl_float3 temp;
	temp.s[0] = lhs.s[0]+rhs.s[0];
	temp.s[1] = lhs.s[1]+rhs.s[1];
	temp.s[2] = lhs.s[2]+rhs.s[2];
	return temp;
}
void executeIntervals(const std::vector<cl_float3> &rays, float *intervals, cl_float3 direction, int steps, float depth) {

	ray_buf = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(rays[0])*rays.size(), NULL, &err);
	if(err < 0) {
		perror("Couldn't create a memory object");
		exit(1);   
	}
	clEnqueueWriteBuffer(queue, ray_buf, CL_FALSE, 0, sizeof(rays[0])*rays.size(), &rays[0], 0, NULL, NULL);

	interval_buf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 
			sizeof(float)*rays.size()*steps, NULL, &err);
	if(err < 0) {
		perror("Couldn't create a memory object");
		exit(1);   
	};

	cl_int err;
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &ray_buf);
	if(err < 0) {
		printf("Couldn't set kernel arguments 0 %d\n", err);
		exit(1);   
	}
	err = clSetKernelArg(kernel, 1, sizeof(cl_float3), &direction);
	if(err < 0) {
		printf("Couldn't set kernel arguments 1 %d\n", err);
		exit(1);   
	}
	err = clSetKernelArg(kernel, 2, sizeof(int), &steps);
	if(err < 0) {
		printf("Couldn't set kernel arguments 2 %d\n", err);
		exit(1);   
	}
	err = clSetKernelArg(kernel, 3, sizeof(float), &depth);
	if(err < 0) {
		printf("Couldn't set kernel arguments 3 %d\n", err);
		exit(1);   
	}
	err = clSetKernelArg(kernel, 4, sizeof(cl_mem), &interval_buf);
	if(err < 0) {
		printf("Couldn't set kernel arguments 4 %d\n", err);
		exit(1);   
	}
	size_t worksizes[2] = { rays.size(), steps };
	err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, worksizes, NULL, 0, NULL, NULL);

	err = clEnqueueReadBuffer(queue, interval_buf, CL_TRUE, 0, sizeof(float)*rays.size()*steps, intervals, 0, NULL, NULL);
	if(err < 0) {
		printf("Couldn't Read Buffer %d\n", err);
		exit(1);   
	}
	clFinish(queue);
	clReleaseMemObject(ray_buf);
	clReleaseMemObject(interval_buf);

}
void analyseIntervals(
		const std::vector<cl_float3> &ray_start, 
		cl_float3 direction,
		int nsteps, 
		float depth, 
		float *intervals,
		std::vector<cl_float3> *result_rays,
		std::vector<cl_float3> *surface_points 
		) {

	for(unsigned int i = 0; i<ray_start.size(); i++) {
		for(int j = 0; j<nsteps; j++) {
			if(intervals[i*nsteps+j] * intervals[i*nsteps+j+1] < 0 ) {

				cl_float3 interval_start;
				float stepsize = depth/(float)nsteps; 
				interval_start = ray_start[i]+direction*stepsize*j;

				if( fabs(intervals[i*nsteps+j]) - fabs(intervals[i*nsteps+j+1]) < EPSILON && fabs(intervals[i*nsteps+j]) - fabs(intervals[i*nsteps+j+1]) > -EPSILON ) { 
					cl_float3 interval_end = interval_start+direction*stepsize;
					cl_float3 end_point = (interval_start+interval_end)/2;
					surface_points->push_back(end_point);
					continue;
				}
				result_rays->push_back(interval_start);
			}
		}
	}
}



void evaluateRays(
		const std::vector<cl_float3> &rays,
		cl_float3 direction, 
		int steps, 
		float depth, 
		std::vector<cl_float3> *surface_points) {

	if (rays.empty())
		return;

	float intervals[rays.size()*steps];
	std::vector<cl_float3> result_rays;

	executeIntervals(rays, intervals, direction, steps, depth);
	analyseIntervals(rays, direction, steps, depth, intervals, &result_rays, surface_points);
	evaluateRays(result_rays, direction, steps, depth/(float)steps, surface_points);
}



int main(int argc, char* argv[]) {

	if(argc != 4) {
		printf("Usage: ./program <xres> <yres> <steps>\n");
		exit(1);
	}
	unsigned int xres = atoi(argv[1]);
	unsigned int yres = atoi(argv[2]);
	unsigned int nsteps = atoi(argv[3]);

	//force compilation = no caching
	//setenv("CUDA_CACHE_DISABLE", "1", 1);

	/* OpenCL data structures */
	/* Data and buffers */

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



	std::vector<cl_float3> rays; 
	for(unsigned int i =0; i<xres; i++) {
		for(unsigned int j =0; j<yres; j++) {
			cl_float3 temp;
			temp.x = i/((float)xres-1)*2-1;
			temp.y = j/((float)yres-1)*2-1;
			temp.z = -1.f;
			std::cout<<temp.x<<" "<<temp.y<<" "<<temp.z<<'\n';
			rays.push_back(temp);
		}
	}
	cl_float3 direction;
	direction.x = 0.f;
	direction.y = 0.f;
	direction.z = 1.f;

	std::vector<cl_float3> points;
	evaluateRays(rays, direction, nsteps, 2.f, &points);

	for(unsigned int i = 0; i < points.size(); i++) {
		std::cout<<points[i].x<<" "<<points[i].y<<" "<<points[i].z<<'\n';
	}



	/* Deallocate resources */
	clReleaseKernel(kernel);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);
	return 0;
}

