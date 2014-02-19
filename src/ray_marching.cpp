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
#include "scene.hpp"
#include "renderer.hpp"
#include "light.hpp"
#include "viewer.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GL/gl.h>
#include <CL/cl_gl.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>

#define PROGRAM_FILE "ray_march.cl"
#define WIDTH 1280
#define HEIGHT 720

struct settings sets = {(char*)"Ray_marching",
	SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
	WIDTH, HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN }; 

SDL_GLContext ctx = NULL;


using glm::mat4;
using glm::vec3;
void initGL() {

}

int ticks;
void updateLight(mat4 *_pos, float delta) {
	ticks++;
	*_pos = glm::rotate(0.001f * (float)ticks, vec3(0.f, 1.f, 0.f)) * (*_pos);
}

void updateLight2(mat4 *_pos, float delta) {
	ticks++;
	*_pos = glm::rotate(0.002f * (float)ticks, vec3(0.f, 1.f, 0.f)) * (*_pos);
}

void run(int _tex_w, int _tex_h, int _n_inters, int _ntiles) {

	SDL_Window *win = setGLContext(&ctx, &sets);
	if( win == NULL ) { printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() ); }

	CL_Resources &res = CL_Resources::getInstance();

	cl_program program;
	res.initOpenCLGLContext();
	createProgramFromSource(&res.device, &res.context,  PROGRAM_FILE, &program);
	res.addKernel(&program, "ray_intervals");

	//init glew
	glewExperimental = GL_TRUE;
	GLenum gl_err =glewInit();
	if (gl_err != GLEW_OK) {
		perror("Couldn't initialize GLEW");
		exit(1);
	}  

	//mat4 position =  glm::rotate(-30.f, vec3(1, 0, 0))* glm::scale(20.f, 20.f, 1.0f) * glm::translate(0.f, 0.f, 15.f);
	GLuint shader = create_shader((char*)"pointcloud.vert", (char*)"pointcloud.frag");

	mat4 view = glm::lookAt(vec3(30.0, 15.0, 10.0), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
	mat4 model = mat4(1.0);
	mat4 projection = glm::perspective(70.f, (float)WIDTH/HEIGHT, 0.3f, 100.0f);

	Viewer v(view, projection, model, (int)shader);

	mat4 position =glm::rotate(-30.f, vec3(1.f, 0.f, 0.f)) * glm::scale(20.f, 20.f, 1.f) * glm::translate(0.f, 0.f, 20.f);
	Light light1(position, _tex_w, _tex_h, &updateLight);
	light1.color = glm::vec3(0.0, 0.6, 0.6);
	mat4 position2 = glm::rotate(30.f, vec3(1.f, 0.f, 0.f)) * glm::scale(20.f, 20.f, 1.f) * glm::translate(0.f, 0.f, -20.f) * glm::rotate(180.f, vec3(0, 1, 0));
	Light light2(position2, _tex_w, _tex_h, &updateLight2);

	std::vector<Light*> lights; 
	lights.push_back(&light1);
	//lights.push_back(&light2);
	Scene scene(lights);
	Renderer r(&scene);;

	
	std::function<void(float)> func1 = scene.update;
	std::function<void(float)> func2 = v.update;

	std::vector<std::function<void(float)>> functions;
	functions.push_back(func1);
	functions.push_back(func2);

	float* points[2] = {&v.x_diff, &v.y_diff};
	mainloop(win, &initGL, functions, r.render, points);
}

