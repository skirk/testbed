#include <SDL2/SDL.h>
#include "SDLUtil.h"
#include "OpenCLUtil.h"
#include "OpenGLUtil.h"
#include "resources.hpp"
#include "sampler.hpp"
#include "sample.hpp"
#include "film.hpp"
#include "camera.hpp"
#include "ray.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GL/gl.h>
#include <CL/cl_gl.h>
#include <iostream>
#include <vector>
#include <sys/time.h>

#define PROGRAM_FILE "ray_march.cl"
#define WIDTH 1280
#define HEIGHT 720

struct settings sets = {(char*)"Ray_marching",
	SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
	WIDTH, HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN }; 

struct OpenCLHandle handle;
SDL_GLContext ctx = NULL;

cl_program program;
GLuint texture;
GLuint vao;
GLuint vao2;
CL_Resources res;

void interrupt(std::string, int);
void generate_intervals(struct OpenCLHandle*, std::vector<float> *_intervals, int _nSamples, int _iterations);
void init_buffers(struct OpenCLHandle*, const std::vector<Ray> &_rays, int, int);
void generate_rays(CameraSample *, std::vector<Ray> *_rays, const Camera&, const int, const int);
void generate_raydata(const std::vector<Ray> &_rays, std::vector<vec3> *_renderdata);
void init_gl_buffers(const std::vector<vec3> &_raydata, const std::vector<vec3> &_pointdata);
void analyse_intervals(const std::vector<float> &_intervals,std::vector<Ray> *_rays, std::vector<vec3> *_pointdata); 
timespec diff(timespec start, timespec end);

int depth = 30;
int size, psize;

GLint timeloc, colorloc;

void initGL() {

	using glm::mat4;
	GLuint shader = create_shader((char*)"pointcloud.vert", (char*)"pointcloud.frag");
	glUseProgram(shader);

	mat4 view = glm::lookAt(vec3(40.0, 10.0, 2.0), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
	mat4 model = mat4(1.0);
	mat4 projection = glm::perspective(70.f, (float)WIDTH/HEIGHT, 0.3f, 100.0f);
	
	GLint mv_loc = glGetUniformLocation(shader, "mv");
	GLint mvp_loc = glGetUniformLocation(shader, "mvp");
	timeloc = glGetUniformLocation(shader, "deltatime");
	colorloc = glGetUniformLocation(shader, "cl");
	if (colorloc < 0 ) {
		interrupt("wrong shader", 0);
	}
	mat4 mv = view * model;
	mat4 mvp = projection * mv;

	glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, &mvp[0][0]); 
	glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &mv[0][0]); 


}


void draw(float _time) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUniform1f(timeloc, _time);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices);
	glm::vec4 color = glm::vec4(0.3, 0.0, 0.0, 1.0);
	/*
	glUniform1f(timeloc, _time);
	glUniform4fv(colorloc,1, &color[0]);
	glBindVertexArray(vao);
	glDrawArrays(GL_LINES, 0, size);
	*/

	glBindVertexArray(vao2);
	color = glm::vec4(0.0, 0.6, 0.6, 1.0);
	glUniform4fv(colorloc,1, &color[0]);
	glPointSize(0.001);
	glDrawArrays(GL_POINTS, 0, psize);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

}



void run(int _tex_w, int _tex_h, int _n_inters, int _ntiles) {
	using glm::mat4;
	using glm::vec3;
	
	SDL_Window *win = setGLContext(&ctx, &sets);
	if( win == NULL ) { printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() ); }

	initOpenCLGLContext(&handle);

	//init glew
	glewExperimental = GL_TRUE;
	GLenum gl_err =glewInit();
	if (gl_err != GLEW_OK) {
		perror("Couldn't initialize GLEW");
		exit(1);
	}  

	createProgramFromSource(&handle, PROGRAM_FILE, &program);
	res.addKernel(&program, "ray_intervals");

	Sampler samp(0, _tex_w, 0, _tex_h);
	Film film;
	film.xResolution = _tex_w, film.yResolution = _tex_h;
	float screen[4] = { -_tex_w/_tex_h, _tex_w/_tex_h, -1.0f, 1.0f };
	mat4 position =  glm::rotate(-30.f, vec3(1, 0, 0))* glm::scale(20.f, 20.f, 1.0f) * glm::translate(0.f, 0.f, 15.f);
	mat4 ortho =  glm::scale(1.f, 1.f, -1.f) * glm::translate(0.f, 0.f, 0.0f);
	OrthoCamera cam(position, ortho, screen, &film);


	std::vector<vec3> pointdata;
	std::vector<vec3> raydata;

	std::vector<Sampler*> samplers;
	samp.getSubSamplers(&samplers, _ntiles);

	timespec before, after;
	clock_gettime(CLOCK_REALTIME, &before);
	for(unsigned int i = 0; i < samplers.size(); i++) {

		std::vector<Ray> rays;
		int dx = samplers[i]->dx;
		int dy = samplers[i]->dy;
		CameraSample *arr = samplers[i]->sampleForEachPixel();
		generate_rays(arr, &rays, cam, dx, dy);
		init_buffers(&handle, rays, dx*dy, _n_inters);

		//generate interval data
		std::vector<float> intervals;
		generate_intervals(&handle, &intervals, dx*dy, _n_inters);
		//generate points according to data
		analyse_intervals(intervals, &rays, &pointdata);

		//render data for rays
		//generate_raydata(rays, &raydata);
		res.releaseMemory("result_buf");
		res.releaseMemory("from_buf");
		delete[] arr;
	}
	clock_gettime(CLOCK_REALTIME, &after);
	std::cout<<"time elapsed : "<<diff(before, after).tv_sec<<":"<<diff(before,after).tv_nsec<<'\n';

	std::cout<<"points "<<pointdata.size()<<'\n';
	//std::cout<<"pointdata size "<<sizeof(vec3)*pointdata.size()<<" b"<<'\n';
	std::cout<<"pointdata size "<<sizeof(vec3)*pointdata.size()/1000000<<" mb"<<'\n';
	init_gl_buffers(raydata, pointdata);


	//res.release();

	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	//init_buffers(&handle);
	//execute_kernel(&handle);
	//glBindTexture(GL_TEXTURE_2D, texture);


	mainloop(win, &initGL, &draw);
}

void analyse_intervals(const std::vector<float> &_intervals, std::vector<Ray> *_rays, std::vector<vec3> *_pointdata) {

	int samples = _rays->size();
	int iterations = _intervals.size()/samples;
	for(int i = 0; i< samples; i++) {
		for (int k = 0; k < iterations-1; k++) {
			if( _intervals[i*iterations+k] * _intervals[i*iterations+k+1] <= 0 ) {

				cl_float3 from_vec = (*_rays)[i].m_point; 
				cl_float3 dir_vec =  (*_rays)[i].m_dir;   
				cl_float3 temp2 = {{ dir_vec.x*depth, dir_vec.y*depth, dir_vec.z*depth }};
				cl_float3 end_p = {{ from_vec.x + temp2.x , from_vec.y + temp2.y, from_vec.z + temp2.z }};  
				int step_id = k;
				cl_float3 temp =  {{ end_p.x - from_vec.x, end_p.y - from_vec.y , end_p.z - from_vec.z }};
				float length = sqrt(temp.x*temp.x + temp.y*temp.y + temp.z*temp.z);
				float step = length / iterations;
				cl_float3 final_p ={{ from_vec.x + dir_vec.x*step*step_id, from_vec.y + dir_vec.y*step*step_id, from_vec.z + dir_vec.z*step*step_id}};
				_pointdata->push_back(vec3(final_p.x, final_p.y, final_p.z)); 
			}
		}
	}
}

void init_gl_buffers(const std::vector<vec3> &_raydata, const std::vector<vec3> &_pointdata) {

	glGenVertexArrays(1, &vao);
	glGenVertexArrays(1, &vao2);

	GLuint VBO_ID, VBO_ID2;

	glBindVertexArray(vao);
	glGenBuffers(1, &VBO_ID); 
	glBindBuffer(GL_ARRAY_BUFFER, VBO_ID);	
	size = _raydata.size();
	glBufferData(GL_ARRAY_BUFFER, size*sizeof(vec3), &_raydata[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(0);

	glBindVertexArray(vao2);
	glGenBuffers(1, &VBO_ID2); 
	glBindBuffer(GL_ARRAY_BUFFER, VBO_ID2);	
	psize = _pointdata.size();
	glBufferData(GL_ARRAY_BUFFER, psize*sizeof(vec3), &_pointdata[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

}

void generate_rays(CameraSample *_samples, std::vector<Ray> *_rays, const Camera &_cam, const int _width, const int _height) {

	_rays->resize(_width*_height);
	for(int i = 0; i< _width*_height; i++) {
		_cam.generateRay(_samples[i], &(*_rays)[i]);

	}
}

void generate_raydata(const std::vector<Ray> &_rays, std::vector<vec3> *_renderdata) {

	for(unsigned int i = 0; i < _rays.size(); i++) {
		cl_float3 from_vec = _rays[i].m_point;
		cl_float3 dir_vec = _rays[i].m_dir;
		cl_float3 temp2 = {{ dir_vec.x*depth, dir_vec.y*depth, dir_vec.z*depth }};
		cl_float3 end_p = {{ from_vec.x + temp2.x , from_vec.y + temp2.y, from_vec.z + temp2.z }};  
		_renderdata->push_back(vec3(from_vec.x, from_vec.y, from_vec.z));
		_renderdata->push_back(vec3(end_p.x, end_p.y, end_p.z));
	}
}

void generate_intervals(struct OpenCLHandle *_handle, std::vector<float> *_intervals, int _nSamples, int _iterations) {

	cl_int err;
	err = clSetKernelArg(*res.getKernel("ray_intervals"), 0, sizeof(cl_mem), res.getMemObj("from_buf"));
	if(err < 0) interrupt("Unable to set kernel argument 0 ", err);
	err = clSetKernelArg(*res.getKernel("ray_intervals"), 1, sizeof(int), &depth);
	if(err < 0) interrupt("Unable to set kernel argument 1 ", err);
	err = clSetKernelArg(*res.getKernel("ray_intervals"), 2, sizeof(int), &_iterations);
	if(err < 0) interrupt("Unable to set kernel argument 2", err);
	err = clSetKernelArg(*res.getKernel("ray_intervals"), 3, sizeof(cl_mem), res.getMemObj("result_buf"));
	if(err < 0) interrupt("Unable to set kernel argument 3", err);

	size_t worksize = _nSamples;
	err = clEnqueueNDRangeKernel(handle.queue, *res.getKernel("ray_intervals"), 1, NULL, &worksize, NULL, 0, NULL, NULL);
	if(err < 0) interrupt("Unable to Enqueue Kernel", err);
	clFinish(handle.queue);

	_intervals->resize(_nSamples*_iterations);
	err = clEnqueueReadBuffer(handle.queue, *res.getMemObj("result_buf"), CL_FALSE, 0, sizeof(cl_float)*_nSamples*_iterations, &(*_intervals)[0],  0, NULL, NULL);
	if(err < 0) interrupt("Unable to Enqueue Read Buffer: ", err);
	clFinish(handle.queue);
	std::cout<<"intervals generated\n";
}



void init_buffers(struct OpenCLHandle *_handle, const std::vector<Ray> &_rays, int _n_samples, int _iterations) {

	cl_int err;
	cl_mem *from_buf = new cl_mem(clCreateBuffer(handle.context, CL_MEM_READ_ONLY, sizeof(Ray)*_n_samples, NULL, &err)); 
	if(err < 0) interrupt("Unable to Create a Buffer: ", err);
	res.addMemObj(from_buf, "from_buf");
	err = clEnqueueWriteBuffer(handle.queue, *from_buf, CL_FALSE, 0, sizeof(Ray)*_n_samples, &_rays[0],  0, NULL, NULL);
	if(err < 0) interrupt("Unable to Enqueue Write Buffer: ", err);
	clFinish(handle.queue);
	cl_mem *result_buf = new cl_mem(clCreateBuffer(handle.context, CL_MEM_READ_ONLY, sizeof(cl_float)*_n_samples*_iterations, NULL, &err)); 
	if(err < 0) interrupt("Unable to Create a Buffer: ", err);
	res.addMemObj(result_buf, "result_buf");
	std::cout<<"buffers initialised\n";
}

timespec diff(timespec start, timespec end)
{
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

void interrupt(std::string _errormessage, int _errorcode) {
	std::cout<<_errormessage<<" "<<getCLErrorString(_errorcode)<<'\n';
	res.release();
	exit(1);
}

