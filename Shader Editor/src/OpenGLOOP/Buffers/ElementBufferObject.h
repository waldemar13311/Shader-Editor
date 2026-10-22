#pragma once

#include "../IHaveId.h"
#include <GL/glew.h>

class ElementBufferObject : public IHaveId
{
public:
	ElementBufferObject();
	~ElementBufferObject();

	ElementBufferObject(const ElementBufferObject&) = delete;
	ElementBufferObject& operator=(const ElementBufferObject&) = delete;
	void bind() const;
	void bufferData(GLsizeiptr size, void *data, GLenum drawingType) const;
	void unBind() const;
};
