#include "scene.hpp"
#include "light.hpp"
#include "ray.hpp"
#include "error.hpp"
#include "sampler.hpp"
#include <glm/ext.hpp>

#include <CL/cl.h>


Scene::Scene(std::vector<Light*> _ligths) {

	m_lights = _ligths;
	update = std::bind(&Scene::update_func, this, std::placeholders::_1);

}
void Scene::update_func(float _time) {
	for(unsigned int i =0; i< m_lights.size(); i++) {
		m_lights[i]->update(_time);
	}
}

bool Scene::evaluateLight(std::vector<vec3> *_points, unsigned int count) {
	
	if(count == m_lights.size()) return false;

	CL_Resources &res = CL_Resources::getInstance();
	Sampler samp(0, m_lights[count]->xDim, 0, m_lights[count]->yDim);
	std::vector<LightSample> *samples = samp.sampleForEachPixel();
	RayBatch *batch = m_lights[count]->generateRayBatch(*samples);
	batch->m_depth = 40;
	batch->m_iterations = 200;
	initBuffers(*batch);
	generateIntervals(batch);
	analyseIntervals(*batch, _points);

	delete batch;
	res.releaseMemory("result_buf");
	res.releaseMemory("from_buf");
	return true;
}

void Scene::initBuffers(const RayBatch &_batch) const {

	cl_int err;
	int nRays = _batch.m_rays.size();
	int iterations = _batch.m_iterations;
	CL_Resources &res = CL_Resources::getInstance();
	cl_mem *from_buf = new cl_mem(clCreateBuffer(res.context, CL_MEM_READ_ONLY, sizeof(Ray)*nRays, NULL, &err)); 
	if(err < 0) interrupt("Unable to Create a Buffer: ", err);
	res.addMemObj(from_buf, "from_buf");
	err = clEnqueueWriteBuffer(res.queue, *from_buf, CL_FALSE, 0, sizeof(Ray)*nRays, &_batch.m_rays[0],  0, NULL, NULL);
	if(err < 0) interrupt("Unable to Enqueue Write Buffer: ", err);
	clFinish(res.queue);
	cl_mem *result_buf = new cl_mem(clCreateBuffer(res.context, CL_MEM_READ_ONLY, sizeof(cl_float)*nRays*iterations, NULL, &err)); 
	if(err < 0) interrupt("Unable to Create a Buffer: ", err);
	res.addMemObj(result_buf, "result_buf");
}

void Scene::generateIntervals(RayBatch *_batch) const {

	int depth = _batch->m_depth;
	int iterations = _batch->m_iterations;
	size_t worksize = _batch->m_rays.size();

	CL_Resources &res = CL_Resources::getInstance();
	cl_int err;
	err = clSetKernelArg(*res.getKernel("ray_intervals"), 0, sizeof(cl_mem), res.getMemObj("from_buf"));
	if(err < 0) interrupt("Unable to set kernel argument 0 ", err);
	err = clSetKernelArg(*res.getKernel("ray_intervals"), 1, sizeof(int), &depth);
	if(err < 0) interrupt("Unable to set kernel argument 1 ", err);
	err = clSetKernelArg(*res.getKernel("ray_intervals"), 2, sizeof(int), &iterations);
	if(err < 0) interrupt("Unable to set kernel argument 2", err);
	err = clSetKernelArg(*res.getKernel("ray_intervals"), 3, sizeof(cl_mem), res.getMemObj("result_buf"));
	if(err < 0) interrupt("Unable to set kernel argument 3", err);

	err = clEnqueueNDRangeKernel(res.queue, *res.getKernel("ray_intervals"), 1, NULL, &worksize, NULL, 0, NULL, NULL);
	if(err < 0) interrupt("Unable to Enqueue Kernel", err);
	clFinish(res.queue);

	_batch->m_intervals.resize(worksize*iterations);
	err = clEnqueueReadBuffer(res.queue, *res.getMemObj("result_buf"), CL_FALSE, 0, sizeof(cl_float)*worksize*iterations, &_batch->m_intervals[0],  0, NULL, NULL);
	if(err < 0) interrupt("Unable to Enqueue Read Buffer: ", err);
	clFinish(res.queue);
}

vec3 convertToVec3(const cl_float3 &_f3) {
	return vec3(_f3.s[0], _f3.s[1], _f3.s[2]);
}

void Scene::analyseIntervals(const RayBatch &_batch, std::vector<vec3> *_pointdata) const {
	int depth = _batch.m_depth;
	int samples = _batch.m_rays.size();
	int iterations = _batch.m_intervals.size()/samples;
	for(int i = 0; i< samples; i++) {
		for (int k = 0; k < iterations-1; k++) {
			if( _batch.m_intervals[i*iterations+k] * _batch.m_intervals[i*iterations+k+1] <= 0 ) {
				vec3 from_vec = convertToVec3(_batch.m_rays[i].m_point);
				vec3 dir_vec =  convertToVec3(_batch.m_rays[i].m_dir);
				vec3 end_p = from_vec + dir_vec*(float)depth;
				int step_id = k;
				float length = glm::distance(from_vec, end_p);
				float step = length / iterations;
				vec3 final_p = from_vec + dir_vec*step*(float)step_id; 
				_pointdata->push_back(vec3(final_p.x, final_p.y, final_p.z)); 
			}
		}
	}

}
