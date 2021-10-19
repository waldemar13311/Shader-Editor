#include "ElementBufferObject.h"

ElementBufferObject::ElementBufferObject()
{
	glGenBuffers(1, &id_);
}

void ElementBufferObject::bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_);
}

void ElementBufferObject::bufferData(const GLsizeiptr size, void* data, const GLenum drawingType) const
{
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, drawingType);
}

void ElementBufferObject::unBind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
