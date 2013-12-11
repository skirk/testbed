#ifndef _SCENE_H_
#define _SCENE_H_
#include <vector>
#include <glm/glm.hpp>
#include "ray.hpp"
#include <functional>

using glm::vec3;
class Light;
class Scene {
	public:
		Scene(std::vector<Light*> _lighst);
		bool evaluateLight(std::vector<vec3> *_points, unsigned int count);
		std::function<void(float)> update;
		std::vector<Light*> m_lights;
	private:
		//functions
		void update_func(float _time);
		void generateIntervals(RayBatch *_batch) const;
		void initBuffers(const RayBatch &_batch) const;
		void analyseIntervals(const RayBatch &_batch, std::vector<vec3> *_pointdata) const;
		mutable bool m_init;

		//variables
};

#endif
