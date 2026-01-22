#pragma once
#include <glad/glad.h>

namespace Renderer {
	class VertexArray {
	public:
		unsigned int ID;

		VertexArray() {
			glGenVertexArrays(1, &ID);
		}
		~VertexArray() {
			glDeleteVertexArrays(1, &ID);
		}

		void bind() const {
			glBindVertexArray(ID);
		}
		void unbind() const {
			glBindVertexArray(0);
		}

		void linkAttribute(const VertexBuffer &vbo, unsigned int iLayoutIndex,
			int iNumComponents, int iStride, int iOffset) {
			bind();
			vbo.bind();
			glVertexAttribPointer(iLayoutIndex, iNumComponents, GL_FLOAT, GL_FALSE,
				iStride * sizeof(float), (void*)(iOffset * sizeof(float)));
			glEnableVertexAttribArray(iLayoutIndex);
			vbo.unbind();
		}
	};
}