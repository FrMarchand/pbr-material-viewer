#include "cubemap.h"
#include <stb_image.h>
#include <iostream>

CubeMap::CubeMap()
{
	glGenTextures(1, &mID);
}

CubeMap::~CubeMap()
{
	release();
}

GLuint CubeMap::getId() const
{
	return mID;
}

void CubeMap::bind(GLenum textureUnit) const
{
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mID);
}

void CubeMap::release()
{
	glDeleteTextures(1, &mID);
	mID = 0;
}