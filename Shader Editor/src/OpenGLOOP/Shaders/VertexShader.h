#pragma once

#include "Shader.h"


class VertexShader : public Shader
{
public:
	explicit VertexShader();

	explicit VertexShader(const char * code);

	std::string getInfoLog() override;
};

