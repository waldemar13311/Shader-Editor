#pragma once

#include "../IHaveId.h"
#include <string>

#include "Shader.h"

class ShaderProgram : public IHaveId
{
public:
	ShaderProgram();

	void attachShader(const Shader& shader) const;
	bool link() const;
	void use() const;
	void deleteProgram() const;

	std::string getInfoLog() const;

	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setFloat(const std::string &name, float value) const;
	void setVec2(const std::string &name, float x, float y) const;
	void setVec3(const std::string &name, float x, float y, float z) const;
	void setVec4(const std::string &name, float x, float y, float z, float w) const;

	//void setMat2(const std::string &name, const glm::mat2 &mat) const
	//{
	//	glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	//}
	//// ------------------------------------------------------------------------
	void setMat4(const std::string &name, const float *value) const;
	void setMat3(const std::string &name, const float *value) const;


};

