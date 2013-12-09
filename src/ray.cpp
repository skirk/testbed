#include "ray.hpp"

Ray::Ray(const vec3 &_point, const vec3 &_dir) {
	m_point.s[0] = _point.x;
	m_point.s[1] = _point.y;
	m_point.s[2] = _point.z;
       	m_dir.s[0] = _dir.x;
       	m_dir.s[1] = _dir.y;
       	m_dir.s[2] = _dir.z;
}

