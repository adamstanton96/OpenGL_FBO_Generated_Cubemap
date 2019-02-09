//move.h
//Repurposed from code found in the "advanced graphics programming" lab base codes.
#ifndef MOVE
#define MOVE

#define DEG_TO_RADIAN 0.017453293

#include <GL\glew.h>
#include <glm\glm.hpp>

namespace move
{
	glm::vec3 forward(glm::vec3 pos, GLfloat angle, GLfloat d);
	glm::vec3 right(glm::vec3 pos, GLfloat angle, GLfloat d);
}
#endif