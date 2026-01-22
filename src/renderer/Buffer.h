#pragma once
#include <glad/glad.h>

namespace Renderer {
	class VertexBuffer {
	public:
		unsigned int ID;

		VertexBuffer(const void * data, unsigned int uiSize) {
			glGenBuffers(1, &ID);
			glBindBuffer(GL_ARRAY_BUFFER, ID);
			glBufferData(GL_ARRAY_BUFFER, uiSize, data, GL_STATIC_DRAW);
		}
		~VertexBuffer() {
			glDeleteBuffers(1, &ID);
		}

		void bind() const {
			glBindBuffer(GL_ARRAY_BUFFER, ID);
		}
		void unbind() const {
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	};
}