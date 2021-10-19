#include "Shader.h"
#include <stdexcept>

Shader::Shader(const ShaderType typeOfShader)
	: code_(nullptr)
{
	id_ = glCreateShader(typeOfShader);
}

Shader::Shader(const ShaderType typeOfShader, const char* code)
	: Shader(typeOfShader)
{
	setCode(code);
}

void Shader::setCode(const char* code)
{
	code_ = code;
	glShaderSource(id_, 1, &code_, nullptr);
}

bool Shader::compile() const
{
	glCompileShader(id_);
	int success;
	glGetShaderiv(id_, GL_COMPILE_STATUS, &success);

	if(!success)
		throw std::runtime_error("Bad compile");

	return  success;
}

void Shader::deleteShader() const
{
	glDeleteShader(id_);
}

std::string Shader::getInfoLog()
{
	int logLen;
	glGetShaderiv(id_, GL_INFO_LOG_LENGTH, &logLen);

	if(logLen > 0)
	{
		const auto infoLog = new char[logLen];
		glGetShaderInfoLog(id_, logLen, nullptr, infoLog);

		return infoLog;
	}

	return nullptr;
}
