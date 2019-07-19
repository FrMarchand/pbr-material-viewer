#include "texture.h"
#include <stb_image.h>
#include <iostream>
#include <vector>
#include <cmrc\cmrc.hpp>
CMRC_DECLARE(resources);

Texture::Texture()
{
	glGenTextures(1, &mID);
}

Texture::Texture(const GLchar* texturePath, bool srgb)
{
	glGenTextures(1, &mID);
	loadTexture(texturePath, srgb);
}

Texture::~Texture()
{
	release();
}

GLuint Texture::getId() const
{
	return mID;
}

void Texture::bind(GLenum textureUnit) const
{
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, mID);
}

void Texture::loadTexture(const GLchar* texturePath, bool srgb)
{
	glBindTexture(GL_TEXTURE_2D, mID);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(false);
	unsigned char *data = stbi_load(texturePath, &width, &height, &nrChannels, 4);

	if (!data) {
		// Fall back to embedded resources
		auto fs = cmrc::resources::get_filesystem();
		if (fs.exists(texturePath)) {
			auto shaderRes = fs.open(texturePath);
			std::string str(shaderRes.begin(), shaderRes.size());
			data = stbi_load_from_memory((const unsigned char*)str.c_str(), str.size(), &width, &height, &nrChannels, 4);
		}
	}

	if (data) {
		GLenum format = GL_RGBA;

		if (srgb) {
			format = GL_SRGB_ALPHA;
		}
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);
}

void Texture::release()
{
	glDeleteTextures(1, &mID);
	mID = 0;
}