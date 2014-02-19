#define FUNCTION t_decocube

#include "models.h"
__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;


__kernel void ray_intervals(
		__global float *points,
		__global float *result) {

	float3 point = (float3)(points[get_global_id(0)*3], points[get_global_id(0)*3+1], points[get_global_id(0)*3+2]);
	result[get_global_id(0)] = FUNCTION(point);
}

