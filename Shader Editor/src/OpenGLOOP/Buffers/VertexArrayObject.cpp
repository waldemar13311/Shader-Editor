#include "VertexArrayObject.h"

#include "GLEW/glew.h"


VertexArrayObject::VertexArrayObject()
{
	glGenVertexArrays(1, &id_);
}

void VertexArrayObject::bind() const
{
	glBindVertexArray(id_);
}

void VertexArrayObject::unBind() const
{
	glBindVertexArray(0);
}

