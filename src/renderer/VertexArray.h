#pragma once
#include <glad/glad.h>
#include "Buffer.h"

namespace Renderer {
class VertexArray {
public:
    unsigned int ID;

    VertexArray() { glGenVertexArrays(1, &ID); }
    ~VertexArray() { glDeleteVertexArrays(1, &ID); }

    VertexArray(const VertexArray &) = delete;
    VertexArray &operator=(const VertexArray &) = delete;

    void Bind() const { glBindVertexArray(ID); }
    void Unbind() const { glBindVertexArray(0); }

    void linkAttribute(const VertexBuffer &vbo,
                       unsigned int iLayoutIndex,
                       int iNumComponents,
                       int iStride,
                       int iOffset) {
        Bind();
        vbo.Bind();
        glEnableVertexAttribArray(iLayoutIndex);
        glVertexAttribPointer(iLayoutIndex,
                              iNumComponents,
                              GL_FLOAT,
                              GL_FALSE,
                              iStride * sizeof(float),
                              (void *)(iOffset * sizeof(float)));
        vbo.Unbind();
    }
};
}  // namespace Renderer