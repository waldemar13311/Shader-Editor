#pragma once

#include "../IHaveId.h"
#include <GLEW/glew.h>

// VBO
class VertexBufferObject : public IHaveId
{
public:
	VertexBufferObject();
	void bind() const;
	void unBind() const;

	void bufferData(GLsizeiptr size, void *data, GLenum drawingType) const;

	void vertexAttribPointer(
		GLuint index,
		const void * offset,
		GLint size,
		GLsizei stride,
		GLenum typeOfDate,
		GLboolean isNormalized
	) const;

	void vertexAttribPointer(
		GLuint progId,
		const char* nameOfAttribute,
		const void * offset,
		GLint size,
		GLsizei stride,
		GLenum typeOfDate,
		GLboolean isNormalized
	);
};

