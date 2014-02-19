#include <GL/glew.h>
#include <iostream>
#include "renderer.hpp"
#include "scene.hpp"
#include "light.hpp"
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>
using glm::mat4;
using glm::vec4;
using glm::vec3;

Renderer::Renderer(Scene *_scene): m_scene(_scene) {

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);
	glBindVertexArray(m_vao);
	render = std::bind(&Renderer::render_func, this);
	glBindVertexArray(0);

}

Renderer::~Renderer() {
	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);
}

void Renderer::render_func() {

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	int i = 0;
	std::vector<vec3> points;
	while( m_scene->evaluateLight(&points, i)) {
		i++;
		/*
		for(unsigned int i = 0; i < points.size(); i++) {
			std::cout<<glm::to_string(points[i])<<'\n';
		}
		*/
		int colorloc = glGetUniformLocation(3, "color");
		vec3 color(1.f-(i*0.5), 1.f-(i*0.2), 1.f-(i*0.4));
		glUniform3fv(colorloc, 1, &color[0]);
		updateGLBuffer(m_vbo, points);
		glBindVertexArray(m_vao);
		glDrawArrays(GL_POINTS, 0, points.size());
		points.clear();
	}
}

void Renderer::updateGLBuffer(GLuint _buffer, const std::vector<vec3> &_points) {

	glBindVertexArray(m_vao);
	glGenBuffers(1, &m_vbo); 
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);	
	glBufferData(GL_ARRAY_BUFFER, _points.size()*sizeof(_points[0]), &_points[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
