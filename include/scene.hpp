#ifndef _SCENE_H_
#define _SCENE_H_
#include <vector>
#include <glm/glm.hpp>
#include "ray.hpp"

using glm::vec3;
class Light;
class Scene {
	public:
		Scene(std::vector<Light*> _lighst);
		void update(float _time);
		void evaluateLights(std::vector<vec3> *_points);
	private:
		//functions
		void generateIntervals(RayBatch *_batch) const;
		void initBuffers(const RayBatch &_batch) const;
		void analyseIntervals(const RayBatch &_batch, std::vector<vec3> *_pointdata) const;

		//variables
		std::vector<Light*> m_lights;
};

#endif
