#include "scene.hpp"
#include "light.hpp"
#include "ray.hpp"
#include "error.hpp"

#include <CL/cl.h>

void Scene::update(float _time) {
	for(unsigned int i =0; i< m_lights.size(); i++) {
		m_lights[i]->update(_time);
	}
}

void Scene::evaluateLights(std::vector<vec3> *_points) {
	for(unsigned int i =0; i< m_lights.size(); i++) {
		std::vector<Ray> rays;
		//m_lights[i]->generateRays
	}
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
	err = clEnqueueReadBuffer(res.queue, *res.getMemObj("result_buf"), CL_FALSE, 0, sizeof(cl_float)*worksize*iterations, &(*_batch).m_intervals[0],  0, NULL, NULL);
	if(err < 0) interrupt("Unable to Enqueue Read Buffer: ", err);
	clFinish(res.queue);
	std::cout<<"intervals generated\n";
}

void Scene::analyseIntervals(const RayBatch &_batch, std::vector<vec3> *_pointdata) {
	int depth = _batch.m_depth;
	int samples = _batch.m_rays.size();
	int iterations = _batch.m_intervals.size()/samples;
	for(int i = 0; i< samples; i++) {
		for (int k = 0; k < iterations-1; k++) {
			if( _batch.m_intervals[i*iterations+k] * _batch.m_intervals[i*iterations+k+1] <= 0 ) {

				cl_float3 from_vec = _batch.m_rays[i].m_point; 
				cl_float3 dir_vec =  _batch.m_rays[i].m_dir;   
				cl_float3 temp2 = {{ dir_vec.x*depth, dir_vec.y*depth, dir_vec.z*depth }};
				cl_float3 end_p = {{ from_vec.x + temp2.x , from_vec.y + temp2.y, from_vec.z + temp2.z }};  
				int step_id = k;
				cl_float3 temp =  {{ end_p.x - from_vec.x, end_p.y - from_vec.y , end_p.z - from_vec.z }};
				float length = sqrt(temp.x*temp.x + temp.y*temp.y + temp.z*temp.z);
				float step = length / iterations;
				cl_float3 final_p ={{ from_vec.x + dir_vec.x*step*step_id, from_vec.y + dir_vec.y*step*step_id, from_vec.z + dir_vec.z*step*step_id}};
				_pointdata->push_back(vec3(final_p.x, final_p.y, final_p.z)); 
			}
		}
	}

}
