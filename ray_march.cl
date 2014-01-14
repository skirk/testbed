#include "models.h"
#define FUNCTION t_shikki
__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;


__kernel void ray_intervals(
		__global float *points,
		__global float *result) {

	float3 point = (float3)(points[get_global_id(0)*3], points[get_global_id(0)*3+1], points[get_global_id(0)*3+2]);
	result[get_global_id(0)] = FUNCTION(point);
}

/*
__kernel void ray_intervals(
		__global float *points,
		__private int depth,
		__private int iterations,
		__global float *result) {


	float3 ray_start = rays[get_global_id(0)*2];
	float3 ray_dir =   rays[get_global_id(0)*2+1];
	float3 ray_end = ray_start + ray_dir*depth;

	float step = length((ray_end - ray_start))/iterations;
	
	bool posFound = false; 

	for(int i = 0; i < iterations; i++) {
		if (posFound)
		{
			result[get_global_id(0)*iterations + i] = 1.0f; 
		}
		else
		{

			float value = FUNCTION(ray_start+ ray_dir*i*step );
			if (value > 0) posFound = true;

			result[get_global_id(0)*iterations + i] = value; 
		}
	}
}
*/

/*
   __kernel void ray_march(
//	__read_only image2d_t from_buf,
//	__read_only image2d_t to_buf,
__private int iterations,
__write_only image2d_t texture)
{

int x = get_global_id(0);
int y = get_global_id(1);
int2 coord = { x,  y };

//float4 from = { 0.0, 0.0, 20.0, 1.0}; //{ 2.0/(float)get_global_size(0)*x -1.0, 2.0/(float)get_global_size(1)*y -1.0,  0.0, 1.0 };
float4 from = { 2.0/(float)get_global_size(0)*x -1.0, 2.0/(float)get_global_size(1)*y -1.0, 1.0, 1.0 };
float4 to =   { 2.0/(float)get_global_size(0)*x -1.0, 2.0/(float)get_global_size(1)*y -1.0,-1.0, 1.0 };
float3 dir = normalize( to.xyz - from.xyz );
float3 pos = from.xyz;
float3 norm = (float3)(0.0, 1.0, 0.0);
float stepsize = distance(to.xyz, from.xyz)/iterations;

int i = 0;
bool inBounds = true;
for(i=0; i < iterations; i++){
float functionValue = FUNCTION(pos);
if( functionValue > 0 ) {
norm = getGradient(pos);
break;
}
pos += stepsize*dir;
inBounds = CheckInBounds(pos);
if( !inBounds ) {
break;
}
}

float4 color;
if( i==iterations ) 
color = (float4)(0.0,0.0,0.0,0.0); 

else {
float4 LightPosition = (float4)(4.0, 0.0, 4.0, 0.0);
float3 LightIntensity = (float3)(0.7, 0.7, 0.7);
float3 Kd = (float3)(0.4, 0.5, 0.3);
float3 Ka = (float3)(0.1, 0.1, 0.1);
float3 Ks = (float3)(0.95, 0.95, 0.95);
float Shininess = 20;

float3 n = normalize( norm );
float3 s = normalize( LightPosition.xyz - pos );
float3 v = normalize( (float3)(pos));
float3 r = -s  -2.0 * dot(n, -s) * n; //reflect(-s, n);
color = (float4)( LightIntensity * (Ka + Kd * max (dot(s,n), 0.0) + Ks *pow(max(dot(r,v), 0.0), Shininess)), 1.0);
//color = (float4)(1.0, 0.0, 0.0, 1.0);
//color = (float4)( 0.0, (float)inBounds, 0.0, 1.0);
//color =(float4)(clamp( ( (float3)(0.88, 0.78, 0.54) * LdotN) *0.9+ 0.1, 0.0, 1.0), 1.0);
}
write_imagef(texture, coord, color);
}


 */






