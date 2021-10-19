#pragma once

#include "../IHaveId.h"

// VAO
class VertexArrayObject : public IHaveId
{
public:
	VertexArrayObject();

	void bind() const;
	void unBind() const;

};

