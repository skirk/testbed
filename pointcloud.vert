#version 430

uniform mat4 mv;
uniform mat3 norm_mat;
uniform mat4 mvp;
uniform float deltatime;
uniform vec3 color;

in vec3 pos;
out vec4 ocl;

mat4 rotationMatrix(vec3 ax, float angle)
{
	vec3 axis = normalize(ax);
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;
	return mat4(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s, 0.0,
			oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s, 0.0,
			oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c, 0.0,
			0.0, 0.0, 0.0, 1.0);
}

void main(void) {

	ocl =vec4(color, 1.0);

	mat4 rotation = rotationMatrix(vec3(0.0,1.0, 0.0), deltatime*0.0001);
	mat4 rotation2 = rotationMatrix(vec3(1.0,0.0, 0.0), deltatime*0.0001);

	//normal = normalize( mat3(rotation) * norm_mat * in_norm ); // vec3(in_norm);
	//position = vec3( mv * rotation * vec4(in_pos, 1.0));

	gl_Position = mvp * rotation * vec4(pos, 1.0);
}
