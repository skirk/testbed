
#define FUNCTION sphere
__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

constant float3 fragboundmin= {-0.5f,-0.5f,-0.5f};
constant float3 fragboundmax = {0.5f,0.5f,0.5f};
float sphere(float3 pos) {
	return 1.0 - pos.x*pos.x - pos.y*pos.y - pos.z*pos.z;
}
float noisesphere(float3 pos)
{
	float x = pos.x;///5;
	float y = pos.y;///5;
	float z = pos.z;///5;
	float tparam = 0.0;
	float ct = cos(tparam);
	float st = sin(tparam);

	float x1 = x*ct*10.+y*st*10.;
	float y1 = -x*st*10.+y*ct*10.;
	float z1 = z*10.;
	float sphere = 81.0 - x1*x1 - y1*y1 - z1*z1;

	float a1 = 3.8, a2 = 1.5, phase = 1.1;
	a1 = 3.8+tparam;

	float a2x = x1*a2;
	float a2y = y1*a2;
	float a2z = z1*a2;
	float sx = sin(a2x);
	float sy = sin(a2y);
	float sz = sin(a2z);
	float a1d = a1/1.17;
	float sx2 = sin(a2x/1.35+phase*sz)*a1d*0.5;
	float sy2 = sin(a2y/1.35+phase*sx)*a1d*0.4;
	float sz2 = sin(a2z/1.35+phase*sy)*a1d*0.8;
	float Serx = a1*sx + sx2;
	float Sery = a1*sy + sy2;
	float Serz = a1*sz + sz2;
	float SS = Serx*Sery*Serz;
	return sphere+SS;
}

float3 getGradient(float3 pos)
{
	float delta = 0.001f;
	float3 ret;
	ret.x = FUNCTION(pos+(float3)(delta,0.0,0.0)) - FUNCTION(pos-(float3)(delta,0.0,0.0));
	ret.y = FUNCTION(pos+(float3)(0.0,delta,0.0)) - FUNCTION(pos-(float3)(0.0,delta,0.0));
	ret.z = FUNCTION(pos+(float3)(0.0,0.0,delta)) - FUNCTION(pos-(float3)(0.0,0.0,delta));

	return normalize(-ret);
}

bool CheckInBounds(float3 pos)
{
	return (pos.x<fragboundmax.x)&&(pos.y<fragboundmax.y)&&(pos.z<fragboundmax.z)&&(pos.x>fragboundmin.x)&&(pos.y>fragboundmin.y)&&(pos.z>fragboundmin.z);
}


__kernel void ray_intervals(
		__global float3 *from_buf,
		__global float3 *dir_buf,
		__private int depth,
		__private int iterations,
		__global float *result) {


	float3 ray_start = from_buf[get_global_id(0)];
	float3 ray_dir = dir_buf[get_global_id(0)];
	//float3 direction = rays[get_global_id(0)*2 +1];
	float3 ray_end = ray_start + ray_dir*depth;

	float step = length((ray_end - ray_start))/iterations;

	for(int i = 0; i < iterations; i++) {
		float value = FUNCTION(ray_start+ ray_dir*i*step );
		result[get_global_id(0)*iterations + i] = value; //(float3)(get_global_id(0), 0.0, 0.0);
	//result[get_global_id(0)*2 +1] = ray_dir; //(float3)(get_global_id(0), 0.0, 0.0);
	}
}


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
/*
		   inBounds = CheckInBounds(pos);
		   if( !inBounds ) {
		   break;
		   }
*/
	}

	float4 color;
	if( i==iterations /*|| !inBounds */) 
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








