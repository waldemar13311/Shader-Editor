#include "ShaderProgram.h"

#include "GLEW/glew.h"

#include <stdexcept>

ShaderProgram::ShaderProgram()
{
	id_ = glCreateProgram();
}

void ShaderProgram::attachShader(const Shader& shader) const
{
	glAttachShader(id_, shader.getId());
}

bool ShaderProgram::link() const
{
	glLinkProgram(id_);
	int success;
	glGetProgramiv(id_, GL_LINK_STATUS, &success);

	if(!success)
		throw std::runtime_error("Bad link program");

	return success;
}

void ShaderProgram::use() const
{
	glUseProgram(id_);
}

void ShaderProgram::deleteProgram() const
{
	glDeleteProgram(getId());
}

std::string ShaderProgram::getInfoLog() const
{
	int logLen;
	glGetProgramiv(id_, GL_INFO_LOG_LENGTH, &logLen);
	if (logLen > 0)
	{
		const auto infoLog = new char[logLen];
		glGetProgramInfoLog(id_, logLen, nullptr, infoLog);

		return infoLog;
	}

	return nullptr;
}

void ShaderProgram::setBool(const std::string &name, const bool value) const
{
	glUniform1i(glGetUniformLocation(id_, name.c_str()), static_cast<int>(value));
}

void ShaderProgram::setInt(const std::string &name, const int value) const
{
	glUniform1i(glGetUniformLocation(id_, name.c_str()), value);
}

void ShaderProgram::setFloat(const std::string &name, const float value) const
{
	glUniform1f(glGetUniformLocation(id_, name.c_str()), value);
}

void ShaderProgram::setVec2(const std::string &name, const float x, const float y) const
{
	glUniform2f(glGetUniformLocation(id_, name.c_str()), x, y);
}

void ShaderProgram::setVec3(const std::string &name, const float x, const float y, const float z) const
{
	glUniform3f(glGetUniformLocation(id_, name.c_str()), x, y, z);
}

void ShaderProgram::setVec4(const std::string &name, const float x, const float y, const float z, const float w) const
{
	glUniform4f(glGetUniformLocation(id_, name.c_str()), x, y, z, w);
}

void ShaderProgram::setMat3(const std::string &name, const float *value) const
{
	glUniformMatrix3fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, value);
}

void ShaderProgram::setMat4(const std::string &name, const float *value) const
{
	glUniformMatrix4fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, value);
}

