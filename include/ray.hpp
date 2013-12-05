#ifndef _RAY_H_
#define _RAY_H_
#include <CL/cl.h>
#include <glm/glm.hpp>

using glm::vec3;

struct Ray {
	public:
		Ray() {}
		Ray(const vec3 &_point,const vec3 &_dir);
		cl_float3 m_point;
		cl_float3 m_dir;
};
		

#endif
