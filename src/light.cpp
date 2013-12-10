#include "light.hpp"
#include "ray.hpp"
#include "sample.hpp"
#include "film.hpp"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

using glm::vec4;
float Light::generateRay(const LightSample &sample, Ray *ray) const {

	vec4 cameraspace =  vec4( 2 * (sample.imageX + 0.5) /xDim -1 ,
				  1 - 2 * (sample.imageY + 0.5 ) / yDim,
				  0.f, 1.f);
	vec4 worldspace = LightToWorld * cameraspace;
	vec4 direction = LightToWorld * vec4(0, 0, -1, 0); 

	*ray = Ray(vec3(worldspace), vec3(direction));
	return 1.0;

}

RayBatch *Light::generateRayBatch(const std::vector<LightSample> &_samples) const {
	RayBatch *batch = new RayBatch();
	batch->m_rays.resize(_samples.size());
	for(unsigned int i=0;i <_samples.size(); i++) {
		generateRay( _samples[i], &batch->m_rays[i]); 
	} 
	return batch;
}

void Light::update(float _deltatime) {

	funcPtr(&(*this).LightToWorld, _deltatime);
}

