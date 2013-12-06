#ifndef _RENDERER_H_
#define _RENDERER_H_
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <vector>

class Scene;

using glm::vec3;
class Renderer {
	public:
		Renderer();
		~Renderer();
		void render(Scene *_scene);
	private:
		void updateGLBuffer(GLuint _buffer, const std::vector<vec3> &_points);

		GLuint m_vao;
		GLuint m_vbo;
};

#endif
