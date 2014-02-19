#ifndef _LIGHT_H_
#define _LIGHT_H_

#include <glm/glm.hpp>
#include <vector>

using glm::mat4;
using glm::vec3;

class LightSample;
class Ray;
class RayBatch;

class Light {

	public:

		typedef void (*updateFunc)(mat4*, float _deltatime);

		Light(const mat4 &_light2world, int _xDim, int _yDim, updateFunc _func) :  xDim(_xDim), yDim(_yDim), LightToWorld(_light2world) {
			funcPtr = _func;
			color = vec3(1.0, 0.0, 0.0);	
		}
		float generateRay(const LightSample &sample, Ray *ray) const;
		RayBatch *generateRayBatch(const std::vector<LightSample> &_samples) const;
		void update(float _deltatime);
		int xDim, yDim;
		vec3 color;
	private:
		updateFunc funcPtr;
		mat4 LightToWorld;
};

#endif

