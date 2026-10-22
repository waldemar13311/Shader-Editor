#include "VertexArrayObject.h"

#include <GL/glew.h>


VertexArrayObject::VertexArrayObject()
{
	glGenVertexArrays(1, &id_);
}

VertexArrayObject::~VertexArrayObject()
{
	if (id_ != 0)
		glDeleteVertexArrays(1, &id_);
}

void VertexArrayObject::bind() const
{
	glBindVertexArray(id_);
}

void VertexArrayObject::unBind() const
{
	glBindVertexArray(0);
}

