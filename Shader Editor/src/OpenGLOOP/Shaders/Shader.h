#pragma once

#include "../IHaveId.h"
#include "ShaderType.h"

#include <string>


class  Shader : public IHaveId
{
public:
	explicit Shader(ShaderType typeOfShader);
	Shader(ShaderType typeOfShader, const char * code);

	void setCode(const char* code);
	bool compile() const;
	void deleteShader() const;
	
	virtual std::string getInfoLog();


private:
	const char* code_;
};
