#pragma once

#include "Shader.h"

class FragmentShader : public Shader
{
public:
	explicit FragmentShader();

	explicit FragmentShader(const char * code);

	std::string getInfoLog() override;
};

