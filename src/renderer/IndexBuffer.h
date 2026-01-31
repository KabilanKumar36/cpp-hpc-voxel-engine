#pragma once

#include <glad/glad.h>

namespace Renderer {
	class IndexBuffer {
	public:
		unsigned int ID;
		unsigned int m_uiCount;

		IndexBuffer(unsigned int* data, unsigned int uiCount) : m_uiCount(uiCount) {
			glGenBuffers(1, &ID);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_uiCount * sizeof(unsigned int),
				data, GL_STATIC_DRAW);
		}
		~IndexBuffer() {
			glDeleteBuffers(1, &ID);
		}
		void Bind() {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
		}
		void Unbind() {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(const IndexBuffer&) = delete;
	};
}