#ifndef _RAY_H_
#define _RAY_H_
#include <glm/glm.hpp>

using glm::vec3;

class Ray {
	public:
		Ray(vec3 _point, vec3 _dir);
		vec3 m_point;
		vec3 m_dir;
};
		

#endif
