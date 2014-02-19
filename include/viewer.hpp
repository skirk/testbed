#pragma once
#include <glm/glm.hpp>
#include <functional>

using glm::mat4;
using glm::vec4;

class Viewer {

	public:
		Viewer(const mat4 &_view, const mat4 &_projection, const mat4 &_model, int _shader);
		
		std::function<void(float)> update;
		mat4 view;
		mat4 projection;
		mat4 model;
		float x_diff, y_diff;
	private:
		void update_func(float delta);
		void setMatrices();
		vec4 position;
		int mv_loc, mvp_loc, m_shader;


};
