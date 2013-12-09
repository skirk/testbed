#ifndef _RAY_H_
#define _RAY_H_
#include <CL/cl.h>
#include <glm/glm.hpp>
#include <vector>

using glm::vec3;

struct Ray {
	public:
		Ray() {}
		Ray(const vec3 &_point,const vec3 &_dir);
		vec3 m_point;
		vec3 m_dir;
};
		
struct RayBatch {
	std::vector<Ray> m_rays;
	std::vector<float> m_intervals;
	int m_depth;
	int m_iterations;
};

#endif
