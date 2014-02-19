#include "OpenGLUtil.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define normalize(x, y, z)                  \
{                                               \
        float norm = 1.0f / sqrt(x*x+y*y+z*z);  \
        x *= norm; y *= norm; z *= norm;        \
}
#define PI 3.1415926f
#define I(_i, _j) ((_j)+4*(_i))

char* read_file(const char* filename, size_t* size) {

	FILE *handle;
	char *buffer;

	/* Read program file and place content into buffer */
	handle = fopen(filename, "r");
	if(handle == NULL) {
		perror("Couldn't find the file");
		exit(1);
	}
	fseek(handle, 0, SEEK_END);
	*size = (size_t)ftell(handle);
	rewind(handle);
	buffer = (char*)malloc(*size+1);
	buffer[*size] = '\0';
	fread(buffer, sizeof(char), *size, handle);
	fclose(handle);

	return buffer;
}


void compile_shader(GLint shader) {

	GLint success;
	GLsizei log_size;
	GLchar *log;

	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
		log = (char*) malloc(log_size+1);
		log[log_size] = '\0';
		glGetShaderInfoLog(shader, log_size+1, NULL, log);
		printf("%s\n", log);
		free(log);
		exit(1);
	}   
}

/* Create, compile, and deploy shaders */
GLuint create_shader(char *_vertex, char *_fragment) {

	printf("Entering Function\n");
	GLuint vs, fs, prog;
	char *vs_source, *fs_source;
	size_t vs_length, fs_length;

	vs = glCreateShader(GL_VERTEX_SHADER);
	fs = glCreateShader(GL_FRAGMENT_SHADER);   

	printf("Shaders created\n");
	vs_source = read_file(_vertex, &vs_length);
	fs_source = read_file(_fragment, &fs_length);

	printf("Files read\n");
	glShaderSource(vs, 1, (const char**)&vs_source, (GLint*)&vs_length);
	glShaderSource(fs, 1, (const char**)&fs_source, (GLint*)&fs_length);
	compile_shader(vs);
	compile_shader(fs);

	printf("Shaders compiled\n");
	prog = glCreateProgram();

	glBindAttribLocation(prog, 0, "in_coords");
	glBindAttribLocation(prog, 1, "in_color");

	glAttachShader(prog, vs);
	glAttachShader(prog, fs);

	printf("Use shader\n");
	glLinkProgram(prog);
	glUseProgram(prog);
	return prog;
}

void matrixSetIdentityM(float *m)
{
        memset((void*)m, 0, 16*sizeof(float));
        m[0] = m[5] = m[10] = m[15] = 1.0f;
}

void matrixTranslateM(float *m, float x, float y, float z)
{
        for (int i = 0; i < 4; i++)
        {
                m[12+i] += m[i]*x + m[4+i]*y + m[8+i]*z;
        }
}

void matrixLookAtM(float *m,
                float eyeX, float eyeY, float eyeZ,
                float cenX, float cenY, float cenZ,
                float  upX, float  upY, float  upZ)
{
        float fx = cenX - eyeX;
        float fy = cenY - eyeY;
        float fz = cenZ - eyeZ;
        normalize(fx, fy, fz);
        float sx = fy * upZ - fz * upY;
        float sy = fz * upX - fx * upZ;
        float sz = fx * upY - fy * upX;
        normalize(sx, sy, sz);
        float ux = sy * fz - sz * fy;
        float uy = sz * fx - sx * fz;
        float uz = sx * fy - sy * fx;

        m[ 0] = sx;
        m[ 1] = ux;
        m[ 2] = -fx;
        m[ 3] = 0.0f;
        m[ 4] = sy;
        m[ 5] = uy;
        m[ 6] = -fy;
        m[ 7] = 0.0f;
        m[ 8] = sz;
        m[ 9] = uz;
        m[10] = -fz;
        m[11] = 0.0f;
        m[12] = 0.0f;
        m[13] = 0.0f;
        m[14] = 0.0f;
        m[15] = 1.0f;
        matrixTranslateM(m, -eyeX, -eyeY, -eyeZ);
}
void matrixMultiplyMM(float *m, float *lhs, float *rhs)
{
        float t[16];
        for (int i=0; i < 4; i++) {
                register const float rhs_i0 = rhs[I(i, 0)];
                register float ri0 = lhs[ I(0,0) ] * rhs_i0;
                register float ri1 = lhs[ I(0,1) ] * rhs_i0;
                register float ri2 = lhs[ I(0,2) ] * rhs_i0;
                register float ri3 = lhs[ I(0,3) ] * rhs_i0;
                for (int j = 1; j < 4; j++) {
                        register const float rhs_ij = rhs[ I(i,j) ];
                        ri0 += lhs[ I(j,0) ] * rhs_ij;
                        ri1 += lhs[ I(j,1) ] * rhs_ij;
                        ri2 += lhs[ I(j,2) ] * rhs_ij;
                        ri3 += lhs[ I(j,3) ] * rhs_ij;
                }
                t[ I(i,0) ] = ri0;
                t[ I(i,1) ] = ri1;
                t[ I(i,2) ] = ri2;
                t[ I(i,3) ] = ri3;
        }
        memcpy(m, t, sizeof(t));
}
void matrixSetProjectionM(float *m, float fov, float aspect, 
		float znear, float zfar)
{
		const float h = 1.0f/tan(fov*0.5*PI/180);
		float neg_depth = znear-zfar;

		m[0] = h / aspect;
		m[1] = 0;
		m[2] = 0;
		m[3] = 0;

		m[4] = 0;
		m[5] = h;
		m[6] = 0;
		m[7] = 0;

		m[8] = 0;
		m[9] = 0;
		m[10] = (zfar + znear)/neg_depth;
		m[11] = -1;

		m[12] = 0;
		m[13] = 0;
		m[14] = 2.0f*(znear*zfar)/neg_depth;
		m[15] = 0;
}
char *gl_error_to_string(GLenum _err) {

	char *error;

	switch(_err) {
		case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
		case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
		case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
		case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
		default: error="NO_PROBLEM"; break;
	}

	//cerr << "GL_" << error.c_str() <<" - "<<file<<":"<<line<<endl;
	return error;
}

