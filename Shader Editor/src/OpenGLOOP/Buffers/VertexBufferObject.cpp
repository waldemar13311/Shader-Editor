#include "VertexBufferObject.h"

#include "GLEW/glew.h"

VertexBufferObject::VertexBufferObject()
{
	glGenBuffers(1, &id_);
}

void VertexBufferObject::bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, id_);
}

void VertexBufferObject::unBind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBufferObject::bufferData(const GLsizeiptr size, void* data, const GLenum drawingType) const
{
	glBufferData(GL_ARRAY_BUFFER, size, data, drawingType);
}

void VertexBufferObject::vertexAttribPointer(
	const GLuint index, 
	const void* offset,
	const GLint size,
	const GLsizei stride,
	const GLenum typeOfDate, 
	const GLboolean isNormalized) const
{
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, size, typeOfDate, isNormalized, stride, offset);
}

void VertexBufferObject::vertexAttribPointer(
	GLuint progId, 
	const char* nameOfAttribute, 
	const void* offset, 
	GLint size,
	GLsizei stride, 
	GLenum typeOfDate, 
	GLboolean isNormalized)
{
	const auto indexLocation = glGetAttribLocation(progId, nameOfAttribute);
	vertexAttribPointer(indexLocation, offset, size, stride, typeOfDate, isNormalized);
}
