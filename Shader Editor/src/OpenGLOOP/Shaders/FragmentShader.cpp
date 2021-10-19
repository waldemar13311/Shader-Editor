#include "FragmentShader.h"

FragmentShader::FragmentShader()
	: Shader(ShaderType::FRAGMENT)
{
}

FragmentShader::FragmentShader(const char* code)
	: Shader(ShaderType::FRAGMENT, code)
{
}

std::string FragmentShader::getInfoLog()
{
	std::string res = "\nERROR::SHADER_COMPILATION_ERROR of type: FRAGMENT\n";
	res.append(Shader::getInfoLog());
	res.append("-----------------------------------------------------\n");

	return res;
}
