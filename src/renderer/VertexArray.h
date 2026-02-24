#pragma once
#include <glad/glad.h>
#include "Buffer.h"
#include "IndexBuffer.h"

namespace Renderer {

/**
 * @class VertexArray
 * @brief Wrapper for OpenGL VAO (Vertex Array Object).
 * Stores the configuration of vertex attributes (layout) and binds VBOs to shader inputs.
 */
class VertexArray {
public:
    unsigned int m_RendererID;

    VertexArray() { glCreateVertexArrays(1, &m_RendererID); }
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
        // 1. Enable the attribute index on the VAO explicitly
        glEnableVertexArrayAttrib(m_RendererID, iLayoutIndex);

        // 2. Specify the FORMAT of the data (Components, Type, Normalized, Relative Offset)
        glVertexArrayAttribFormat(m_RendererID,
                                  iLayoutIndex,
                              iNumComponents,
                              GL_FLOAT,
                              GL_FALSE,
                                  (void *)(uintptr_t)(iOffset * sizeof(float)));

        // 3. Attach the VBO to a "Binding Point" on the VAO.
        // We use iLayoutIndex as the binding point index for simplicity.
        // Signature: (VAO_ID, BindingIndex, VBO_ID, BufferStartOffset, Stride)
        glVertexArrayVertexBuffer(
            m_RendererID, iLayoutIndex, vbo.m_RendererID, 0, iStride * sizeof(float));

        // 4. Link the Attribute Index to that Binding Point
        glVertexArrayAttribBinding(m_RendererID, iLayoutIndex, iLayoutIndex);
    }
    /**
     * @brief Attaches an IndexBuffer to this VAO using DSA.
     * @param ibo The IndexBuffer to attach.
     */
    void AttachIndexBuffer(const IndexBuffer &ibo) const {
        glVertexArrayElementBuffer(m_RendererID, ibo.m_RendererID);
    }
};
}  // namespace Renderer