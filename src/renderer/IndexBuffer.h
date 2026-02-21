#pragma once

#include <glad/glad.h>

namespace Renderer {
class IndexBuffer {
public:
    unsigned int m_RendererID;
    unsigned int m_uiCount;

    /**
     * @brief Creates and fills the index buffer.
     * @param data Pointer to the indices array.
     * @param uiCount Total NUMBER of indices (not bytes).
     */
    IndexBuffer(unsigned int* data, unsigned int uiCount) : m_uiCount(uiCount) {
        glCreateBuffers(1, &m_RendererID);
        glNamedBufferData(m_RendererID, uiCount * sizeof(unsigned int), data, GL_STATIC_DRAW);
    }

    ~IndexBuffer() { glDeleteBuffers(1, &m_RendererID); }

    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    void Bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID); }
    void Unbind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

    unsigned int GetCount() const { return m_uiCount; }
};
}  // namespace Renderer