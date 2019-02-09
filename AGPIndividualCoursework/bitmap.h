//bitmap.h
//Repurposed from code found in the "advanced graphics programming" lab base codes.
#ifndef BITMAP
#define BITMAP

#include <SDL_ttf.h>
#include <GL/glew.h>
#include <iostream>

namespace bitmap
{
	GLuint loadBitmap(char *filename);
}

#endif
