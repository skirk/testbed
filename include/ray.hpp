#ifndef _RAY_H_
#define _RAY_H_
#include <glm/glm.hpp>
#include <CL/cl.h>
#include <vector>

using glm::vec3;

struct Ray {
	public:
		Ray() {}
		Ray(const vec3 &_point,const vec3 &_dir);
		cl_float3 m_point;
		cl_float3 m_dir;
};
		
struct RayBatch {
	std::vector<Ray> m_rays;
	std::vector<float> m_intervals;
	int m_depth;
	int m_iterations;
};

#endif
