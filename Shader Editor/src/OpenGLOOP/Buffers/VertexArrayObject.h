#pragma once

#include "../IHaveId.h"

// VAO
class VertexArrayObject : public IHaveId
{
public:
	VertexArrayObject();
	~VertexArrayObject();

	VertexArrayObject(const VertexArrayObject&) = delete;
	VertexArrayObject& operator=(const VertexArrayObject&) = delete;

	void bind() const;
	void unBind() const;

};

