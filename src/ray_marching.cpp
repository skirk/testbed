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

#define PROGRAM_FILE "ray_march.cl"
#define WIDTH 1280
#define HEIGHT 720
#define TXT_W 60	
#define TXT_H 60

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

int indices[] = { 0, 1, 2, 0, 2, 3};
void interrupt(std::string, int);
void generate_intervals(struct OpenCLHandle*, std::vector<float> *_intervals, int _nSamples, int _iterations);
void init_buffers(struct OpenCLHandle*, int, int);
void generate_rays(const int, const int);
void init_gl_buffers(const std::vector<vec3> &_raydata, const std::vector<vec3> &_pointdata);
void analyse_intervals(const std::vector<float> &_intervals, std::vector<vec3> *_raydata, std::vector<vec3> *_pointdata); 

int depth = 30;
int size, psize;

GLint timeloc, colorloc;
std::vector<Ray> rays;

void initGL() {

	using glm::mat4;
	GLuint shader = create_shader((char*)"pointcloud.vert", (char*)"pointcloud.frag");
	glUseProgram(shader);

	mat4 view = glm::lookAt(vec3(40.0, 20.0, 2.0), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
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
	//GLint textureLoc = glGetUniformLocation(shader, "diffuseTex");
	//glUniform1i(textureLoc, 0);
	
	
	/*
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	GLuint VBOs[3];
	float pos[] = {
		-1.0f,  -1.0f,
		-1.0f,  1.0f,
		1.0f,  1.0f,
		1.0f, -1.0f};
	float uv[] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f};
	glGenBuffers(3, VBOs);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*8, pos, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*8, uv, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	*/

	/*
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOs[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(char)*6, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	*/


}


void draw(float _time) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUniform1f(timeloc, _time);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices);
	glUniform1f(timeloc, _time);
	glm::vec4 color = glm::vec4(0.3, 0.0, 0.0, 1.0);
	glUniform4fv(colorloc,1, &color[0]);
	glBindVertexArray(vao);
	glDrawArrays(GL_LINES, 0, size);

	glBindVertexArray(vao2);
	color = glm::vec4(0.0, 1.0, 1.0, 1.0);
	glUniform4fv(colorloc,1, &color[0]);
	glPointSize(5.0);
	glDrawArrays(GL_POINTS, 0, psize);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

}



void run(int _tex_w, int _tex_h, int _n_inters) {
	
	SDL_Window *win = setGLContext(&ctx, &sets);
	if( win == NULL ) { printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() ); }

	initOpenCLGLContext(&handle);

	//init glew
	glewExperimental = GL_TRUE;
	GLenum gl_err =glewInit();
	GLenum GLERR;
	if (gl_err != GLEW_OK) {
		perror("Couldn't initialize GLEW");
		exit(1);
	}  

	createProgramFromSource(&handle, PROGRAM_FILE, &program);
	res.addKernel(&program, "ray_march");
	res.addKernel(&program, "ray_intervals");

	generate_rays(_tex_w, _tex_h);


	init_buffers(&handle, _tex_w*_tex_h, _n_inters);
	std::vector<float> intervals;
	generate_intervals(&handle, &intervals, _tex_w*_tex_h, _n_inters);

	std::vector<vec3> raydata;
	raydata.resize(_tex_w*_tex_h);
	std::vector<vec3> pointdata;
       	analyse_intervals(intervals, &raydata, &pointdata);
	
	init_gl_buffers(raydata, pointdata);

	//res.release();

	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	//init_buffers(&handle);
	//execute_kernel(&handle);
	//glBindTexture(GL_TEXTURE_2D, texture);


	mainloop(win, &initGL, &draw);
}

void analyse_intervals(const std::vector<float> &_intervals, std::vector<vec3> *_raydata, std::vector<vec3> *_pointdata) {

	int samples = _raydata->size();
	int iterations = _intervals.size()/samples;
	for(int i = 0; i< samples; i++) {
		cl_float3 from_vec =rays[i].m_point; //from[i];
		cl_float3 dir_vec = rays[i].m_dir; //dir[i];
		//std::cout<<"from vec " <<from_vec.x << " " << from_vec.y << " " << from_vec.z <<'\n';
		//std::cout<<"dir vec " <<dir_vec.x << " " << dir_vec.y << " " << dir_vec.z <<'\n';
		cl_float3 temp2 = { dir_vec.x*depth, dir_vec.y*depth, dir_vec.z*depth };
		cl_float3 end_p = { from_vec.x + temp2.x , from_vec.y + temp2.y, from_vec.z + temp2.z };  

		_raydata->push_back(vec3(from_vec.x, from_vec.y, from_vec.z));
		_raydata->push_back(vec3(end_p.x, end_p.y, end_p.z));

		for (int k = 0; k < iterations; k++) {
			if( _intervals[i*iterations+k] * _intervals[i*iterations+k+1] <= 0 ) {
				//std::cout<<[i*iterations+k] << " "<< array[i*iterations+k+1] << '\n';
				int step_id = k;// (i*iterations+k) % iterations ;
				cl_float3 temp =  { end_p.x - from_vec.x, end_p.y - from_vec.y , end_p.z - from_vec.z };
				float length = sqrt(temp.x*temp.x + temp.y*temp.y + temp.z*temp.z);
				float step = length / iterations;
				cl_float3 final_p = { from_vec.x + dir_vec.x*step*step_id, from_vec.y + dir_vec.y*step*step_id, from_vec.z + dir_vec.z*step*step_id};
				_pointdata->push_back(vec3(final_p.x, final_p.y, final_p.z)); 
				//std::cout<<final_p.x << " " << final_p.y << " " << final_p.z <<'\n';
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

void generate_rays( const int _width, const int _height) {

	Sampler samp(0, _width, 0, _height);
	CameraSample *arr = samp.sampleForEachPixel();
	using glm::mat4;
	Film film;
	film.xResolution = _width;
	film.yResolution = _height;
	//float zfar = 100;
	//float znear = 0;
	float screen[4] = { -_width/_height, _width/_height, -1.0f, 1.0f };
	mat4 position = /* glm::rotate(0.f, glm::vec3(1, 0, 0))*/glm::scale(20.f, 20.f, 1.0f) * glm::translate(0.f, 0.f, 15.f); //// //glm::translate(2.f, 2.f, 2.f);
	mat4 ortho =  glm::scale(1.f, 1.f, -1.f) * glm::translate(0.f, 0.f, 0.0f);

	OrthoCamera cam(position, ortho, screen, &film);


	rays.reserve(_width*_height);
	for(int i = 0; i< _width*_height; i++) {
		cam.generateRay(arr[i], &rays[i]);
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



void init_buffers(struct OpenCLHandle *_handle, int _n_samples, int _iterations) {

	cl_int err;
	cl_mem *from_buf = new cl_mem(clCreateBuffer(handle.context, CL_MEM_READ_ONLY, sizeof(Ray)*_n_samples, NULL, &err)); 
	if(err < 0) interrupt("Unable to Create a Buffer: ", err);
	res.addMemObj(from_buf, "from_buf");
	err = clEnqueueWriteBuffer(handle.queue, *from_buf, CL_FALSE, 0, sizeof(Ray)*_n_samples, &rays[0],  0, NULL, NULL);
	if(err < 0) interrupt("Unable to Enqueue Write Buffer: ", err);
	clFinish(handle.queue);
	cl_mem *result_buf = new cl_mem(clCreateBuffer(handle.context, CL_MEM_READ_ONLY, sizeof(cl_float)*_n_samples*_iterations, NULL, &err)); 
	if(err < 0) interrupt("Unable to Create a Buffer: ", err);
	res.addMemObj(result_buf, "result_buf");
	std::cout<<"buffers initialised\n";
}




/*
   void init_buffers(struct OpenCLHandle *_handle) {

   cl_int err;
   cl_image_format format;
   format.image_channel_order = CL_RGBA;
//cl_mem *from_buf = new cl_mem( clCreateImage2D(_handle->context, CL_MEM_READ_ONLY, &format, (size_t)TXT_W, (size_t)TXT_H, 0, 0, &err));


float from[TXT_W*TXT_H][3];
float to[TXT_W*TXT_H][3];
for(int i = 0; i < TXT_W * TXT_H; i++) {
from[i][0] = to[i][0] = (2/TXT_W)*i - 1;
from[i][1] = to[i][1] = (2/TXT_H)*i - 1;
from[i][2] = 1;
to[i][2] = -1;
}

cl_mem *from_buf = new cl_mem(clCreateBuffer(_handle->context, CL_MEM_READ_ONLY, sizeof(float)*10000, NULL, &err)); 
if(err < 0) interrupt("Unable to Create Memory Object ", err);
res.addMemObj(from_buf, "from_buf");
//cl_mem *to_buf = new cl_mem( clCreateImage2D(_handle->context, CL_MEM_READ_ONLY, &format, (size_t)TXT_W, (size_t)TXT_H, 0, 0, &err));
cl_mem *to_buf = new cl_mem(clCreateBuffer(_handle->context, CL_MEM_READ_ONLY, sizeof(cl_mem), NULL, &err)); 
if(err < 0) interrupt("Unable  to Create Memory Object ", err);
res.addMemObj(to_buf, "to_buf");

glGenTextures(1, &texture);
glBindTexture(GL_TEXTURE_2D, texture);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (int)TXT_W, (int)TXT_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
glBindTexture(GL_TEXTURE_2D, 0);


cl_mem *img_data = new cl_mem( clCreateFromGLTexture2D(_handle->context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, texture, &err));
if(err < 0) interrupt("Unable to create from GL buffer: ", err);
res.addMemObj(img_data, "img_data");

}


void execute_kernel(struct OpenCLHandle *_handle) {

cl_int err;
int iteration = 512;
err = clSetKernelArg(*res.getKernel("ray_march"), 0, sizeof(cl_mem), res.getMemObj("from_buf"));
if(err < 0) interrupt("Unable to set kernel argument ", err);
err = clSetKernelArg(*res.getKernel("ray_march"), 1, sizeof(cl_mem), res.getMemObj("to_buf"));
//if(err < 0) interrupt("Unable to set kernel argument ", err);
err = clSetKernelArg(*res.getKernel("ray_march"), 0, sizeof(int), &iteration);
if(err < 0) interrupt("Unable to set kernel argument ", err);
err = clSetKernelArg(*res.getKernel("ray_march"), 1, sizeof(cl_mem), res.getMemObj("img_data"));
if(err < 0) interrupt("Unable to set kernel argument ", err);

err = clEnqueueAcquireGLObjects(_handle->queue, 1, res.getMemObj("img_data"), 0, NULL, NULL);
if(err < 0) interrupt("Couldn't acquire GL objects ", err);

err = clEnqueueNDRangeKernel(_handle->queue, *res.getKernel("ray_march"), 2, NULL, (size_t[]){TXT_W, TXT_H}, NULL, 0, NULL, NULL);
if(err < 0) interrupt("Unable to enqueue kernel ", err);
clFinish(_handle->queue);
}

*/

void interrupt(std::string _errormessage, int _errorcode) {
	std::cout<<_errormessage<<" "<<getCLErrorString(_errorcode)<<'\n';
	res.release();
	exit(1);
}

/*
   timeloc = glGetUniformLocation(shader, "deltatime");
   GLint model_loc = glGetUniformLocation(shader, "mv");
   GLint norm_loc = glGetUniformLocation(shader, "norm_mat");
   GLint mvp_loc = glGetUniformLocation(shader, "mvp");


   float model[16];
   float view[16];
   float mv[16];
   float mvp[16];
   float projection[16];

   matrixSetIdentityM( model);
   matrixSetIdentityM( projection);

   matrixLookAtM(view,
   50, 20, 0.0, //from
   0.0, 0.0, 0.0, //to
   0.0, 1.0, 0.0); //up



   matrixMultiplyMM(mv, view, model);
   glUniformMatrix4fv(model_loc, 1, GL_FALSE, mv); 

   float normal[9] =
   { mv[0], mv[1], mv[2], 
   mv[4], mv[5], mv[6],
   mv[8], mv[9], mv[10] };

   glUniformMatrix3fv(norm_loc, 1, GL_FALSE, normal);

   matrixSetProjectionM(projection, 60.0, (float)HEIGHT/WIDTH, 0.01f, 120.0f);

   matrixMultiplyMM(mvp, projection, mv);
   glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, mvp); 

   glBindBuffer(GL_ARRAY_BUFFER, VBO_ID[0]);
   glEnable(GL_DEPTH_TEST);
   */

