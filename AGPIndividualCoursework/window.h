//window.h
//Repurposed from code found in the "advanced graphics programming" lab base codes.
#ifndef SDLWINDOW
#define SDLWINDOW

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#include <SDL_ttf.h>
#include <iostream>

namespace SDLWindow 
{
	SDL_Window * setupRC(SDL_GLContext &context);
	SDL_Window * setupRC(SDL_GLContext &context, int width, int height);
	void exitFatalError(const char *message);
}

#endif