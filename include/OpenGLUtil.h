#ifndef _OPENGLUTIL_H_
#define _OPENGLUTIL_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <GL/glew.h>
#include <GL/gl.h>

void compile_shader(GLint shader);
GLuint create_shader(char *_vertex, char *_fragment);
char *read_file(const char* filename, size_t* size);
void matrixSetIdentityM(float *m);
void matrixLookAtM(float *m,
                float eyeX, float eyeY, float eyeZ,
                float cenX, float cenY, float cenZ,
                float  upX, float  upY, float  upZ);
void matrixMultiplyMM(float *m, float *lhs, float *rhs);
void matrixSetProjectionM(float *m, float fov, float aspect, float znear, float zfar);
char *gl_error_to_string(GLenum _err);

#ifdef __cplusplus
}
#endif
#endif

