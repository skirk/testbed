

/*
#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <glm/glm.hpp>

using glm::mat4;

class Film;
class CameraSample;
class Ray;

class Camera {
	public:
		Camera(const mat4 &_cam2world, Film *_film)
			: CameraToWorld(_cam2world), m_film(_film) {};
		virtual float generateRay(const CameraSample &sample, Ray *ray) const = 0;

	protected:
		mat4 CameraToWorld;
		Film *m_film;
};

class ProjectiveCamera : public Camera {
	public: 
		ProjectiveCamera(const mat4 &_cam2world, const mat4 &_proj, const float ScreenWindow[4], Film *_film);
	protected:
		mat4 CameraToScreen;
		mat4 ScreenToRaster;
		mat4 RasterToCamera;
		mat4 RasterToScreen;
};


class OrthoCamera : public ProjectiveCamera {

	public:
		OrthoCamera(const mat4 &_cam2world, const mat4 &_proj, const float ScreenWindow[4], Film *_film)
			: ProjectiveCamera(_cam2world, _proj, ScreenWindow, _film) {}

		float generateRay(const CameraSample &sample, Ray *ray) const;
	private:

};



#endif
*/
