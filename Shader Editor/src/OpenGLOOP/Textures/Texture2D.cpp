#include "Texture2D.h"

#include <GLEW/glew.h>
#include <stb_image.h>

#include <stdexcept>

Texture2D::Texture2D(const char *fileName)
{
	glGenTextures(1, &id_);

	data = stbi_load(fileName, &width, &height, &nrComponents, 0);

	if(!data)
		throw std::runtime_error("Bad load texture");
}

void Texture2D::bind() const
{
	glBindTexture(GL_TEXTURE_2D, id_);
}

void Texture2D::texImage2D() const
{
	GLenum format = 0;
	if (nrComponents == 1)
		format = GL_RED;
	else if (nrComponents == 3)
		format = GL_RGB;
	else if (nrComponents == 4)
		format = GL_RGBA;

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture2D::texParameteri(GLenum pName, GLint param) const
{
	glTexParameteri(GL_TEXTURE_2D, pName, param);
}

void Texture2D::freeImage() const
{
	stbi_image_free(data);
}

void Texture2D::activeTexture(const GLenum texture)
{
	glActiveTexture(texture);
}
