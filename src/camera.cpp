#include "camera.hpp"
#include "ray.hpp"
#include "sample.hpp"
#include "film.hpp"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

ProjectiveCamera::ProjectiveCamera(const mat4 &_cam2world, const mat4 &_proj, const float screenWindow[4], Film *_film)
	: Camera(_cam2world, _film), CameraToScreen(_proj)  {

		//glm::mat4 scale1 = glm::scale((float)_film->xResolution, (float)_film->yResolution, 1.0f);
		//glm::mat4 scale2 = glm::scale(1.f/(screenWindow[1] - screenWindow[0]), 
	//			1.f/(screenWindow[2] - screenWindow[3]), 1.f);
		//glm::mat4 translate = glm::translate(-screenWindow[0], -screenWindow[3], 0.f);

		/*
		CameraToScreen = _proj;
		//ScreenToRaster = scale1 * scale2 * translate;
		RasterToScreen = glm::inverse(ScreenToRaster);
		RasterToCamera = glm::inverse(CameraToScreen)*RasterToScreen;
		std::cout<<glm::to_string(CameraToWorld)<<std::endl;
		*/
	}

void ChangeBasis(const mat4 &_matrix, const vec3 &pt, vec3 *_target);

using glm::vec4;
float OrthoCamera::generateRay(const CameraSample &sample, Ray *ray) const {
	vec3 Pras(sample.imageX, sample.imageY, 0);
	vec3 Pcamera;
	float angle = 60;

	float aspect = m_film->xResolution/m_film->yResolution;
	//glm::mat4 scale1 = glm::scale((float)1.f/m_film->xResolution, (float)1.f/m_film->yResolution, 1.f);
	//vec4 normalized =  scale1 * vec4(Pras, 1.0);
	vec4 cameraspace =  vec4( 2 * (sample.imageX + 0.5) /m_film->xResolution -1 ,
				  1 - 2 * (sample.imageY + 0.5 ) / m_film->yResolution,
				  0.f, 1.f);

	//glm::mat4 scale2 = glm::scale( (2.f - 1.f/normalized.x)  /*(float)tan(angle/2.f * M_PI /180) */ , (1.f/normalized.y - 2.f)  /*(float)tan(angle/2.f * M_PI / 180)*/, 1.f);
	//vec4 cameraspace = normalized * scale2;
	//std::cout<<glm::to_string(CameraToWorld)<<'\n';
	vec4 worldspace = CameraToWorld * cameraspace;
	vec4 direction = CameraToWorld * vec4(0, 0, -1, 0); 

	*ray = Ray(vec3(worldspace), vec3(direction));
	//ChangeBasis(RasterToScreen, Pras, &Pcamera);
	//std::cout<<glm::to_string(RasterToCamera)<<std::endl;
	//ChangeBasis(CameraToWorld, ray->m_point, &ray->m_point);
	//std::cout<<glm::to_string(ray->m_point)<<std::endl;
	//ChangeBasis(CameraToWorld, ray->m_dir, &ray->m_dir);
	//std::cout<<glm::to_string(Pcamera)<<std::endl;
	return 1.0;

}

void ChangeBasis(const mat4 &_matrix, const vec3 &_pt, vec3 *_target) { 

	float x = _pt.x; float y = _pt.y; float z = _pt.z;
	//glm::transpose(_matrix);
	_target->x = _matrix[0][0]*x + _matrix[0][1]*y + _matrix[0][2]*z + _matrix[0][3];
	_target->y = _matrix[1][0]*x + _matrix[1][1]*y + _matrix[1][2]*z + _matrix[1][3];
	_target->z = _matrix[2][0]*x + _matrix[2][1]*y + _matrix[2][2]*z + _matrix[2][3];
	float w =    _matrix[3][0]*x + _matrix[3][1]*y + _matrix[3][2]*z + _matrix[3][3];
	if(w != 1.0) *_target / w;
}




