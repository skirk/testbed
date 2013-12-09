#include <GL/glew.h>
#include <iostream>
#include "renderer.hpp"
#include "scene.hpp"
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>
using glm::mat4;
using glm::vec4;

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
	std::vector<vec3> points;
	m_scene->evaluateLights(&points);
	std::cout<<points.size()<<'\n';
	updateGLBuffer(m_vbo, points);
	glBindVertexArray(m_vao);
	int colorloc = glGetUniformLocation(3, "cl");
	glm::vec4 color(0.0, 0.6, 0.6, 1.0);
	glUniform4fv(colorloc,1, &color[0]);
	glBindVertexArray(m_vao);
	//GLfloat p[2];
	//glGetFloatv(GL_POINT_SIZE_RANGE, p);
	//glPointSize(p[0]);
	//for(unsigned int i = 0; i < points.size(); i++) {
	//		std::cout<<glm::to_string(points[i])<<'\n';
	//	}
	glDrawArrays(GL_POINTS, 0, points.size());

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
