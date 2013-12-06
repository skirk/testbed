#include <GL/glew.h>
#include "renderer.hpp"
#include "scene.hpp"
Renderer::Renderer() {

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);
	glBindVertexArray(m_vao);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(0);

}

Renderer::~Renderer() {
	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);
}

void Renderer::render(Scene *_scene) {
	std::vector<vec3> points;
	_scene->evaluateLights(&points);
	updateGLBuffer(m_vbo, points);
}

void Renderer::updateGLBuffer(GLuint _buffer, const std::vector<vec3> &_points) {

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);	
	glBufferData(GL_ARRAY_BUFFER, _points.size()*sizeof(vec3), &_points[0], GL_DYNAMIC_DRAW);

}
