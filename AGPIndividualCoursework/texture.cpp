#include "texture.h"

Texture::Texture()
{
	Texture("");
}

Texture::Texture(char * filename)
{
	loadTexture(filename);
}

void Texture::loadTexture(char * filename)
{
	texture = bitmap::loadBitmap(filename);
}
