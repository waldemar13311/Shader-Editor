#pragma once

#include "../IHaveId.h"

#include <string>
#include <GLEW/glew.h>

class Texture2D : public IHaveId
{
public:
	explicit Texture2D(const char *fileName);
	void bind() const;
	void texImage2D() const;
	void texParameteri(GLenum pName, GLint param) const;
	void freeImage() const;

	static void activeTexture(GLenum texture);

	std::string type;
	std::string path;

	int width{}, height{}, nrComponents{};

	unsigned char *data;
};

