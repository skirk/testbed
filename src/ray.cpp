#include "ray.hpp"

Ray::Ray(const vec3 &_point, const vec3 &_dir) {
	m_point.x = _point.x;
	m_point.y = _point.y;
	m_point.z = _point.z;
	m_dir.x = _dir.x;
	m_dir.y = _dir.y;
	m_dir.z = _dir.z;
}

