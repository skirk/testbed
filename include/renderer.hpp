#ifndef _RENDERER_H_
#define _RENDERER_H_
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <vector>
#include <functional>

class Scene;

using glm::vec3;
class Renderer {
	public:
		Renderer(Scene *_scene);
		~Renderer();
		std::function<void()> render;
	private:
		void updateGLBuffer(GLuint _buffer, const std::vector<vec3> &_points);
		void render_func();

		Scene *m_scene;
		GLuint m_vao;
		GLuint m_vbo;
};

#endif
