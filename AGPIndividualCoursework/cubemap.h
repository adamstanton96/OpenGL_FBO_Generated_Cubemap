//cubemap.h
//Repurposed from code found in the "advanced graphics programming" lab base codes.
#ifndef CUBEMAP
#define CUBEMAP

#include <SDL_ttf.h>
#include <GL/glew.h>
#include <iostream>

namespace cubemap
{
	GLuint load(const char *fname[6], GLuint *texID, GLenum sides[6]);
}

#endif
