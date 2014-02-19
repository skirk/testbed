#include "viewer.hpp"
#include <GL/glew.h>
#include <iostream>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

using glm::vec3;

Viewer::Viewer(const mat4 &_view, const mat4 &_projection, const mat4 &_model, int _shader):
			view(_view), projection(_projection), model(_model), m_shader(_shader)
{
	mv_loc = glGetUniformLocation(m_shader, "mv");
	mvp_loc = glGetUniformLocation(m_shader, "mvp");
	update = std::bind(&Viewer::update_func, this, std::placeholders::_1);
	position = vec4(40.f, 20.f, 0.f, 0.f);

}
void Viewer::update_func(float delta) {

	model =  glm::rotate( (x_diff*0.03f),  vec3(0, 1, 0)) * glm::rotate(-(y_diff*0.03f), vec3(0, 0, 1)) * model;
	setMatrices();

}

void Viewer::setMatrices() {

	mat4 mv = view * model;
	mat4 mvp = projection * mv;
	glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, &mvp[0][0]); 
	glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &mv[0][0]); 
}

