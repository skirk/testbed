#pragma once

#include <stdint.h>
#include <SDL2/SDL.h>
#include <functional>

struct settings {
	char *title;
	int position_x;
	int position_y;
	int width;
	int height;
	uint32_t flags;
};

SDL_Window *setGLContext(SDL_GLContext*, struct settings*);
void mainloop(SDL_Window*, void (*init_function)(), std::function<void(float)> update_func, std::function<void()> draw_func);
void gl_info(void);
void dump_sdl_error( void );

