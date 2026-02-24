#pragma once
#include <glad/glad.h>
#include "Buffer.h"

namespace Renderer {

/**
 * @class VertexArray
 * @brief Wrapper for OpenGL VAO (Vertex Array Object).
 * Stores the configuration of vertex attributes (layout) and binds VBOs to shader inputs.
 */
class VertexArray {
public:
    unsigned int m_RendererID;

    VertexArray() { glGenVertexArrays(1, &m_RendererID); }
    ~VertexArray() { glDeleteVertexArrays(1, &m_RendererID); }

    VertexArray(const VertexArray &) = delete;
    VertexArray &operator=(const VertexArray &) = delete;

    void Bind() const { glBindVertexArray(m_RendererID); }
    void Unbind() const { glBindVertexArray(0); }

    /**
     * @brief Configures a vertex attribute layout.
     * @param vbo The VertexBuffer to read from.
     * @param iLayoutIndex Shader layout location (e.g., layout(location = 0)).
     * @param iNumComponents Number of components per vertex (e.g., 3 for vec3).
     * @param iStride Total number of FLOATS between vertices.
     * @param iOffset Number of FLOATS to skip to reach this attribute.
     */
    void LinkAttribute(const VertexBuffer &vbo,
                       unsigned int iLayoutIndex,
                       int iNumComponents,
                       int iStride,
                       int iOffset) const {
        Bind();
        vbo.Bind();

        // Convert "Float Count" to "Bytes" for OpenGL
        glVertexAttribPointer(iLayoutIndex,
                              iNumComponents,
                              GL_FLOAT,
                              GL_FALSE,
                              static_cast<GLsizei>(iStride * sizeof(float)),
                              (void *)(uintptr_t)(iOffset * sizeof(float)));

        glEnableVertexAttribArray(iLayoutIndex);

        vbo.Unbind();
        // Note: Do not Unbind VAO here, usually done by the caller or Renderer
    }
};
}  // namespace Renderer