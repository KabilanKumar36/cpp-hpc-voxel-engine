#pragma once
#include <glad/glad.h>

namespace Renderer {

/**
 * @class VertexBuffer
 * @brief Wrapper for OpenGL VBO (Vertex Buffer Object).
 * Stores raw vertex data (positions, colors, normals) on the GPU.
 */
class VertexBuffer {
public:
    unsigned int m_RendererID;

    /**
     * @brief Creates and fills the buffer with data.
     * @param data Pointer to the data array.
     * @param uiSize Total size of the data in bytes.
     */
    VertexBuffer(const void* data, unsigned int uiSize) {
        glCreateBuffers(1, &m_RendererID);
        glNamedBufferData(m_RendererID, uiSize, data, GL_STATIC_DRAW);
    }

    ~VertexBuffer() { glDeleteBuffers(1, &m_RendererID); }

    // Prevent copying (VBOs cannot be shared without reference counting)
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    void Bind() const { glBindBuffer(GL_ARRAY_BUFFER, m_RendererID); }
    void Unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }
    /**
     * @brief Updates buffer data without reallocating memory (DSA).
     */
    void UpdateData(const void* data, unsigned int uiSize, unsigned int uiOffset = 0) const {
        glNamedBufferSubData(m_RendererID, uiOffset, uiSize, data);
    }
};
}  // namespace Renderer