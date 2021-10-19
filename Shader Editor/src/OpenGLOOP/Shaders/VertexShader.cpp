#include "VertexShader.h"

VertexShader::VertexShader()
	: Shader(ShaderType::VERTEX)
{
}

VertexShader::VertexShader(const char* code)
	: Shader(ShaderType::VERTEX, code)
{
}

std::string VertexShader::getInfoLog()
{
	std::string res = "\nERROR::SHADER_COMPILATION_ERROR of type: VERTEX\n";
	res.append(Shader::getInfoLog());
	res.append("-----------------------------------------------------\n");

	return res;
}
