#pragma once
#include "bitmap.h"

class Texture
{
public:
	Texture();
	Texture(char* filename);
	~Texture() {}
	GLuint texture;
protected:
	void loadTexture(char* filename);
};