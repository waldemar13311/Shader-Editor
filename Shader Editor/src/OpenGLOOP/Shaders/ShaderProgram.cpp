#include "ShaderProgram.h"

#include <GL/glew.h>

#include <stdexcept>
#include <vector>

ShaderProgram::ShaderProgram()
{
	id_ = glCreateProgram();
}

ShaderProgram::~ShaderProgram()
{
	if (id_ != 0)
		glDeleteProgram(id_);
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
		std::vector<char> infoLog(static_cast<size_t>(logLen));
		glGetProgramInfoLog(id_, logLen, nullptr, infoLog.data());

		return std::string(infoLog.data());
	}

	return {};
}

GLint ShaderProgram::getUniformLocation(const std::string &name) const
{
	const auto it = uniformLocations_.find(name);
	if (it != uniformLocations_.end())
		return it->second;

	const GLint location = glGetUniformLocation(id_, name.c_str());
	uniformLocations_.emplace(name, location);

	return location;
}

void ShaderProgram::setBool(const std::string &name, const bool value) const
{
	glUniform1i(getUniformLocation(name), static_cast<int>(value));
}

void ShaderProgram::setInt(const std::string &name, const int value) const
{
	glUniform1i(getUniformLocation(name), value);
}

void ShaderProgram::setFloat(const std::string &name, const float value) const
{
	glUniform1f(getUniformLocation(name), value);
}

void ShaderProgram::setVec2(const std::string &name, const float x, const float y) const
{
	glUniform2f(getUniformLocation(name), x, y);
}

void ShaderProgram::setVec3(const std::string &name, const float x, const float y, const float z) const
{
	glUniform3f(getUniformLocation(name), x, y, z);
}

void ShaderProgram::setVec4(const std::string &name, const float x, const float y, const float z, const float w) const
{
	glUniform4f(getUniformLocation(name), x, y, z, w);
}

void ShaderProgram::setMat3(const std::string &name, const float *value) const
{
	glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, value);
}

void ShaderProgram::setMat4(const std::string &name, const float *value) const
{
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, value);
}

