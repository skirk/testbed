#ifndef _SDLUTIL_H_
#define _SDLUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <SDL2/SDL.h>

struct settings {
	char *title;
	int position_x;
	int position_y;
	int width;
	int height;
	uint32_t flags;
};

SDL_Window *setGLContext(SDL_GLContext*, struct settings*);
void mainloop(SDL_Window*, void (*init_function)(), void (*update_function)(float), void (*draw_function)());
void gl_info(void);
void dump_sdl_error( void );

#ifdef __cplusplus
}
#endif

#endif
