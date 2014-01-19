#define FUNCTION t_mitchell

#include "models.h"

__kernel void evaluate_intervals(
		__global float3 *in_ray_start, 
		__private float3 direction,
		__private int nsteps,
		__private float depth,
		__global float *result) {

	float3 ray_start = in_ray_start[get_global_id(0)];

	float step=depth/(float)nsteps;
	direction = normalize(direction);

	float value = FUNCTION(ray_start + direction*step*get_global_id(1));
	result[get_global_id(0)*nsteps + get_global_id(1)] = value;
}

