#include "SDLUtil.h"
#include <GL/gl.h>
#include "renderer.hpp"
#include "timer.hpp"
#include <iostream>



SDL_Window *setGLContext(SDL_GLContext *_ctx, struct settings *_settings)
{

	if( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0 )
	{
		printf("Couldn't init SDL");
		return NULL;
	}

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

	SDL_Window *window = NULL;
	window = SDL_CreateWindow( _settings->title, _settings->position_x, _settings->position_y, _settings->width, _settings->height, _settings->flags);
	if(window == NULL )
	{
		dump_sdl_error( );
		return NULL;
	}
	*_ctx = SDL_GL_CreateContext( window );
	if( !_ctx )
	{
		dump_sdl_error( );
		return NULL;
	}

	dump_sdl_error( );

	gl_info( );

	return window;
}



void mainloop(SDL_Window *_win, void (*init_function)(),const std::vector< std::function<void(float)> > &_update_funcs, std::function<void()> draw_func, float* mouse_diff[2])
{


	float deltaTime = 0.0;
	int thisTime = 0;
	int lastTime = 0;
	float fps =0.f;

	/*
	   if (_win == NULL) {
	   printf("Something went wrong");
	   }

	   if(TTF_Init()==-1) {
	   printf("TTF_INIT: %s\n", TTF_GetError());
	   exit(2);
	   }
	   TTF_Font *font;
	   font=TTF_OpenFont("CircleOfDust.ttf", 16);
	   if(!font) {
	   printf("TTF_OpenFont: %s\n", TTF_GetError());
	   }
	   SDL_Color color={0,0,0};
	   SDL_Surface *text_surface;
	   if(!(text_surface=TTF_RenderText_Solid(font,"Hello World!",color))) {
	//handle error here, perhaps print TTF_GetError at least
	} else {
	SDL_BlitSurface(text_surface,NULL,,NULL);
	//perhaps we can reuse it, but I assume not for simplicity.
	SDL_FreeSurface(text_surface);
	}
	*/



	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	SDL_GL_SwapWindow(_win);
	(*init_function)();

	int quit = 0;
	char title[100];

	float x = 0, y =0;
	while(!quit) {
		SDL_Event event;
		thisTime  = SDL_GetTicks();
		deltaTime = (float)(thisTime-lastTime) / 1000;

		if(thisTime % 1000 - 960 > 0) {
			fps = (float)1.f/deltaTime;
			sprintf(title, "FPS %f", fps);
			SDL_SetWindowTitle(_win, title);
		}
		lastTime = thisTime;

		while(SDL_PollEvent(&event) == 1) {
			switch(event.type) {
				case SDL_QUIT:
					printf("Quit\n");
					quit = 1;
					break;
				case SDL_MOUSEBUTTONDOWN:
					x=event.button.x, y=event.button.y;
					break;
				case SDL_MOUSEBUTTONUP:
					*mouse_diff[0] = event.button.x-x;
				       	*mouse_diff[1] = event.button.y-y;
					x=0;y=0;
					break;
				default:
					break;
			}
		}
		timespec time1, time2;

		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
		for(unsigned int i = 0; i < _update_funcs.size(); i++) {
			_update_funcs[i]((float)thisTime);
		}
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
		std::cout<<"update: "<<diff(time1,time2).tv_sec<<":"<<diff(time1,time2).tv_nsec<<std::endl;
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
		draw_func();
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
		std::cout<<"draw: "<<diff(time1,time2).tv_sec<<":"<<diff(time1,time2).tv_nsec<<std::endl;

		SDL_GL_SwapWindow(_win);
	}
	SDL_Quit();
}



void gl_info( void )
{
	const char* renderer     = (const char*) glGetString(GL_RENDERER);
	const char* version      = (const char*) glGetString(GL_VERSION);
	const char* glsl_version = (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION);

	fprintf( stdout, "[GL] Renderer: %s\n", renderer ? renderer : "unknown" );
	fprintf( stdout, "[GL] Version: %s\n", version ? version : "unknown" );
	fprintf( stdout, "[GL] Shading Language: %s\n", glsl_version ? glsl_version : "unknown" );
}


void dump_sdl_error( void )
{
	const char* sdl_error = SDL_GetError( );

	if( sdl_error && *sdl_error != '\0' )
	{
		fprintf( stderr, "[SDL] %s\n", sdl_error );
	}
}


