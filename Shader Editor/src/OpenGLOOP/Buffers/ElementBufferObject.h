#pragma once

#include "../IHaveId.h"
#include <GLEW/glew.h>

class ElementBufferObject : public IHaveId
{
public:
	ElementBufferObject();
	void bind() const;
	void bufferData(GLsizeiptr size, void *data, GLenum drawingType) const;
	void unBind() const;
};
