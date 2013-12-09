
#define FUNCTION t_shikki
__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

constant float3 fragboundmin= {-0.5f,-0.5f,-0.5f};
constant float3 fragboundmax = {0.5f,0.5f,0.5f};
float sphere(float3 pos) {
	return 1.0 - pos.x*pos.x - pos.y*pos.y - pos.z*pos.z;
}
float hfSphere(const float x, const float y, const float z, float cx, float cy, float cz, float r)
{
	float dx = x - cx;
	float dy = y - cy;
	float dz = z - cz;
	return r*r - dx*dx - dy*dy - dz*dz;
}
float hfEllipsoid(const float x, const float y, const float z, float cx, float cy, float cz, float ra, float rb, float rc)
{
	float dx = (x - cx)/ra;
	float dy = (y - cy)/rb;
	float dz = (z - cz)/rc;
	return 1.0 - dx*dx - dy*dy - dz*dz;
}
float hfTorusX(const float x, const float y, const float z, float cx, float cy, float cz, float R, float r)
{
	float dx = x - cx;
	float dy = y - cy;
	float dz = z - cz;
	return r*r - dx*dx - dy*dy - dz*dz - R*R + 2.0*R*sqrt(dy*dy + dz*dz);
}
float hfTorusY(const float x, const float y, const float z, float cx, float cy, float cz, float R, float r)
{
	float dx = x - cx;
	float dy = y - cy;
	float dz = z - cz;
	return r*r - dx*dx - dy*dy - dz*dz - R*R + 2.0*R*sqrt(dx*dx + dz*dz);
}
float r_subtraction(const float a, const float b)
{
	return a-b-sqrt(a*a+b*b);
};
float r_union(const float a, const float b)
{
	return a+b+sqrt(a*a+b*b);
};
float r_intersection(const float a, const float b)
{
	return a+b-sqrt(a*a+b*b);
};
float hfPow4(const float f)
{
	return f*f*f*f;
}
float if2(const float f)
{
	return 1.0/(1.0+exp(f*(-100.0)));
}
void hfRotate3DZ(const float x, const float y, const float z, float *xt, float *yt, float *zt, float theta) //hmmm
{
	float ct = cos(theta);
	float st = sin(theta);
	*xt = x*ct + y*st;
	*yt = y*ct - x*st;
	*zt = z;
}
float hfEllCylY(const float x, const float y, const float z, float cx, float cy, float cz, float fA, float fB)
{
	float xt = (x - cx)/fA;
	float zt = (z - cz)/fB;
	return 1.0 - xt*xt - zt*zt;
}

float hfBlendInt(const float f1, const float f2, float a0, float a1, float a2)
{
	float f1n = f1/a1;
	float f2n = f2/a2;
	float f1f2 = f1n*f1n + f2n*f2n + 1;
	float disp = a0/f1f2;
	float fres = r_intersection(f1,f2);
	float fret = fres + disp;
	return  fret;
}
float hfBlendUni(const float f1, const float f2, float a0, float a1, float a2)
{
	float f1n = f1/a1;
	float f2n = f2/a2;
	float f1f2 = f1n*f1n + f2n*f2n + 1;
	float disp = a0/f1f2;
	float fres = r_union(f1,f2);
	float fret = fres + disp;
	return fret;
}
float hfCylinderX(const float x, const float y, const float z, float cx, float cy, float cz, float r)
{
	float dx = 0.0;
	float dy = y - cy;
	float dz = z - cz;
	return r*r - dx*dx - dy*dy - dz*dz;
}

float hfCylinderZ(const float x, const float y, const float z, float cx, float cy, float cz, float r)
{
	float dx = x - cy;
	float dy = y - cy;
	float dz = 0.0;
	return r*r - dx*dx - dy*dy - dz*dz;
}

float hfCylinderY(const float x, const float y, const float z, float cx, float cy, float cz, float r)
{
	float dx = x - cx;
	float dy = 0.0;
	float dz = z - cz;
	return r*r - dx*dx - dy*dy - dz*dz;
}


float shikki_cup(float x, float y, float z)
{
	float sphere1 = hfSphere(x, y, z, 0.0, 3.5, 0.0, 5.76);
	float sphere2 = hfSphere(x, y, z, 0.0, 3.5, 0.0, 5.56);

	float plane1 = -y - 1.95;
	float plane2 = y + 2.6;
	float slab = r_intersection(plane1, plane2);
	float conus0 = (y-5.4)*(y-5.4)*0.0625 - x*x - z*z;
	float conus1 = r_intersection(conus0, slab);
	float conus2 = (y-4.2)*(y-4.2)*0.0625 - x*x - z*z;
	float leg = r_subtraction(conus1, conus2);
	float chasha = r_subtraction(r_subtraction(sphere1, sphere2), y);
	return r_union(chasha,leg);
}

float shikki_table(float x, float y, float z)
{
	float xx = x*2.0;
	float yy = y*2.0;
	float zz = z*2.0;

	float EC1 = hfEllCylY(xx, yy, zz, 0.0, 0.0, 0.0, 19.9, 55.0);
	float EC2 = hfEllCylY(xx, yy, zz, 0.0, 0.0, 0.0, 65.0, 12.2);
	float plane = yy + 1.0;
	float Stol = r_intersection(r_intersection(hfBlendInt(EC1,EC2,-0.05,1.0,1.0), plane),-yy);
	float EC3 = hfEllCylY(xx, yy, zz, 0.0, 0.0, 0.0, 19.4, 55.0);
	float EC4 = hfEllCylY(xx, yy, zz, 0.0, 0.0, 0.0, 65.0, 11.7);
	float bottom = yy + 0.5;
	float Stolbot = r_intersection(hfBlendInt(EC3, EC4, -0.05, 1.0, 1.0), bottom);
	float d_plane = (yy + 1.0)/2.7 + (xx - 15.0)/0.3;
	float u_plane = -(yy + 1.0)/2.7 - (xx - 15.8)/0.7;
	float z_slab = r_intersection((zz+10.75),(10.75-zz));
	float y_slab = r_intersection((-0.9-yy), (yy+3.7));
	float r_leg = r_intersection(d_plane, r_intersection(u_plane, r_intersection(z_slab, y_slab)));

	d_plane = (yy + 1.0)/2.7 - (xx - 15.0)/0.3;
	u_plane = -(yy + 1.0)/2.7 + (xx + 15.8)/0.7;
	z_slab = r_intersection((zz+10.75),(10.75-zz));
	y_slab = r_intersection((-0.9-yy), (yy+3.7));
	float l_leg = r_intersection(d_plane, r_intersection(u_plane, r_intersection(z_slab, y_slab)));

	return r_union(r_union(r_subtraction(Stol, Stolbot), r_leg), l_leg);	
}
float ifbb(float f)
{
	if (f > 0)
		return f*f;
	else
		return 0.0;
}
float hfBlock(const float x, const float y, const float z, float cx, float cy, float cz, float dx, float dy, float dz)
{
	float xt = (x - cx)*(cx + dx - x);
	float yt = (y - cy)*(cy + dy - y);
	float zt = (z - cz)*(cz + dz - z);
	float ret = r_intersection(xt, yt);
	ret = r_intersection(ret, zt);
	return ret;
}

float shikki_sakepot(float x, float y, float z)
{
	float xt = x - 3.0;
	float yt = y + 3.0;
	float zt = z;

	//main body & stand
	float sphere = hfSphere(xt, yt, zt, 0.0, -0.5, 0.0, 5.5);
	float cyl_n = r_intersection(hfCylinderY(xt, yt, zt, 0.0, -0.5, 0.0, 3.2), (-3.0-yt)); 
	float blend1 = hfBlendUni(sphere, cyl_n, 0.05, 1.0, 1.0);
	float body = hfBlendInt(blend1, yt+5.4, -0.4, 1.0, 1.0);

	float cyl_v = r_intersection(hfCylinderY(xt, yt, zt, 0.0, -0.5, 0.0, 5.5), (yt+0.5)); 
	body = r_union(body, cyl_v);
	float clipsphere = hfSphere(xt, yt, zt, 0.0, -18.5, 0.0, 20.0); 
	float blendbound = yt+0.5;

	float f3 = blendbound; 
	float b0 = -1000.0; 
	float b1 = 0.1;

	float inter = r_intersection(body, clipsphere);
	float w1 = body*body + clipsphere*clipsphere;
	float w2 = ifbb(blendbound);

	float disp = -1000.0*(1.0 - w1/(w1 + 0.1*w2));
	body = inter + disp;

	float lid = r_intersection(hfSphere(xt, yt, zt, 0.0, -5.2, 0.0, 7.0), (yt-1.1));

	//lid handle
	float lid_ball = hfEllipsoid(xt, yt, zt, 0.0, 2.2, 0.0, 0.8, 0.4, 0.8);
	float lid_cyl = r_intersection(r_intersection(hfCylinderY(xt, yt, zt, 0.0, 2.2, 0.0, 0.8), (yt-1.1)), (2.2 - yt));
	float torcut = hfTorusY(xt, yt, zt, 0.0, 1.6, 0.0, 1.1, 0.7);
	float body2 = r_union(body, r_union(lid, r_subtraction(r_union(lid_ball, lid_cyl), torcut)));

	//two ears
	//	float ear1 = 1.0 - hfPow4((xt-4.6)/0.15) - hfPow4((yt-1.3)/1.6) - hfPow4(zt/0.75);
	//	float ear2 = 1.0 - hfPow4((xt+4.6)/0.15) - hfPow4((yt-1.3)/1.6) - hfPow4(zt/0.75);
	float ear1 = hfEllipsoid(xt, yt, zt, 4.6, 1.3, 0.0, 0.15, 1.6, 0.75);
	float ear2 = hfEllipsoid(xt, yt, zt, -4.6, 1.3, 0.0, 0.15, 1.6, 0.75);
	float ear_hole = hfCylinderX(xt, yt, zt, 0.0, 1.9, 0.0, 0.15); 
	float ears = r_union(ear1, ear2);

	//pot handle
	float sel1 = 1.0 - hfPow4(xt/4.45) - hfPow4((yt-1.6)/9.4) - hfPow4(zt/10.0);
	float sel2 = 1.0 - hfPow4(xt/4.15) - hfPow4((yt-1.6)/9.1) - hfPow4(zt/9.7);
	float handle = r_intersection(r_subtraction(sel1, sel2), (0.8*0.8 - zt*zt));
	float h_cut = 1.0 - hfPow4(xt/10.0) - hfPow4((yt-11.0)/9.7) - hfPow4(zt/1.2);
	handle = r_intersection(handle, h_cut);

	//spout
	float xa, ya, za;
	hfRotate3DZ(xt+5.6, yt+0.2, zt, &xa, &ya, &za, -3.14/20.0);
	float xp = xa-5.6; 
	float yp = ya-0.2; 
	float zp = za;
	float spout = 1.0 - xa*xa/100.0 - ya*ya/1.96 - za*za/0.9025;
	hfRotate3DZ(xt+3.0, yt+4.1, zt, &xa, &ya, &za, 3.14/10.0);

	float el_bl = 1.0 - xa*xa/6.25 - (ya-3.9)*(ya-3.9)/18.49 - za*za/0.9025;
	el_bl = r_intersection(el_bl,  0.5 - yt);

	float clipcyl = hfCylinderZ(xt, yt, zt, -7.6, -1.5, 0.0, 2.6);
	float uni = r_union(spout, el_bl);
	w1 = spout*spout + el_bl*el_bl;
	w2 = f3*f3*if2(clipcyl);

	disp = 0.8*(1.0 - w1/(w1+0.03*f3*f3));
	spout = uni + disp;

	float sp_hole = 1.0 - (xp+5.6)*(xp+5.6)/100.0 - (yp+0.2)*(yp+0.2)/1.44 - (zp/0.75)*(zp/0.75);
	sp_hole = r_subtraction(sp_hole, yp+0.4);

	spout = r_subtraction(r_subtraction(r_subtraction(spout,  yp+0.2), sp_hole), ((yp+0.2)-0.8*(xp+10.2)));
	float el_cut = 1.0 - (xp+10.2)*(xp+10.2)/2.56 - (yp+0.2)*(yp+0.2)/0.25 - (zp/0.75)*(zp/0.75);
	spout = r_subtraction(spout, el_cut);

	//	return r_union(body2, ears);
	return r_union(body2, r_union(r_subtraction(r_union(ears, handle), ear_hole), spout));
}

float shikki_stand(float x, float y, float z)
{
	float xx = x/1.4 + 6.5;
	float yy = y/1.4 + 6.5;
	float zz = z/1.4 + 6.5;

	float blok1 = hfBlock(xx, yy, zz, 0.0, 0.0, 0.0, 11.5, 9.7, 11.5);
	float tang = 0.5/1.4;
	float plane1 = -tang*(xx - 11.5)-(yy - 9.2);
	float plane2 = tang*xx-(yy-9.2);
	float plane3 = -tang*(zz-11.5)-(yy-9.2);
	float plane4 = tang*zz-(yy-9.2);

	float blok11 = r_intersection(r_intersection(r_intersection(r_intersection(blok1, plane1), plane2), plane3), plane4);
	float CylZ1 = 1.0-hfPow4((xx-5.75)/4.0)-hfPow4((yy-4.2)/2.3);
	float CylZ2 = 1.0-hfPow4((xx-5.75)/3.2)-hfPow4((yy-4.2)/3.0);
	float hole1 = r_union (CylZ1, CylZ2);
	float CylX3 = 1.0-hfPow4((yy-4.2)/2.3)-hfPow4((zz-5.75)/4.0);
	float CylX4 = 1.0-hfPow4((yy-4.2)/3.2)-hfPow4((zz-5.75)/3.0);
	float hole2 = r_union(CylX3, CylX4);
	float Sphere = hfSphere(xx, yy, zz, 5.75,11.0,5.7, 2.5); 
	float Bottom = hfSphere(xx, yy, zz, 5.75,12.44,5.7,2.525);
	float chavanchik = r_intersection(r_subtraction(Sphere, Bottom), 11.3-yy);
	float blok2 = hfBlock(xx, yy, zz, 0.5, -1.0, 0.5, 10.5, 10.2, 10.5);

	return r_subtraction(r_subtraction(r_subtraction(r_union(chavanchik, blok11), hole1), hole2), blok2);
}

float t_shikki(float3 _pos)
{
	float x = _pos.x;
	float y = _pos.y;
	float z = _pos.z;
	float tparam = 3.0;
	float x1, y1, z1, x2, y2, z2, x3, y3, x4, y4, z4;

	x1 = x+8.0; y1 = y-8.0;
	x2 = x+9.0; y2 = y-17.0; z2 = z+1.1;
	x3 = x-8.0; y3 = y-8.0;
	x4 = x/2.0; y4 = y/2.0; z4 = z/2.0;
	return r_union(shikki_stand(x1,y1,z), r_union(shikki_cup(x2, y2, z2),r_union(shikki_sakepot(x3, y3, z), shikki_table(x4, y4, z4))));
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
		__global float3 *rays,
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






