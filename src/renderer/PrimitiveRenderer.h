#pragma once

#include <vector>
#include "../core/Matrix.h"
#include "Buffer.h"
#include "Shader.h"
#include "VertexArray.h"

namespace Renderer {
class PrimitiveRenderer {
public:
    static void Init() {
        m_pPrimitiveShader = new Shader("../assets/shaders/vertex_SimpleDebug.glsl",
                                        "../assets/shaders/fragment_SimpleDebug.glsl");
        std::vector<float> cubeVertices = {
            0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0,
            0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0,
            0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1,
        };
        m_pCubeVAO = new VertexArray();
        m_pCubeVBO = new VertexBuffer(
            cubeVertices.data(), static_cast<unsigned int>(cubeVertices.size()) * sizeof(float));
        m_pCubeVAO->linkAttribute(*m_pCubeVBO, 0, 3, 0, 0);
        m_pCubeVAO->Unbind();

        float fLineVertices[] = {0, 0, 0, 0, 0, 0};
        m_pLineVBO = new VertexBuffer(fLineVertices, sizeof(fLineVertices));
        m_pLineVAO = new VertexArray();
        m_pLineVAO->linkAttribute(*m_pLineVBO, 0, 3, 0, 0);
    }
    static void Shutdown() {
        if (m_pPrimitiveShader) {
            delete m_pPrimitiveShader;
            m_pPrimitiveShader = nullptr;
        }
        if (m_pCubeVAO) {
            delete m_pCubeVAO;
            m_pCubeVAO = nullptr;
        }
        if (m_pCubeVBO) {
            delete m_pCubeVBO;
            m_pCubeVBO = nullptr;
        }
        if (m_pLineVAO) {
            delete m_pLineVAO;
            m_pLineVAO = nullptr;
        }
        if (m_pLineVBO) {
            delete m_pLineVBO;
            m_pLineVBO = nullptr;
        }
    }
    static void DrawCube(const Core::Vec3& position,
                         const Core::Vec3& size,
                         const Core::Vec3& color,
                         const Core::Mat4& viewProjMatrix) {
        if (!m_pPrimitiveShader)
            return;
        m_pPrimitiveShader->Use();
        Core::Mat4 objModel =
            Core::Mat4::Translation(position) * Core::Mat4::Scale(size.x, size.y, size.z);

        Core::Mat4 ObjMVP = viewProjMatrix * objModel;
        m_pPrimitiveShader->SetMat4("uViewProjection", ObjMVP);
        m_pPrimitiveShader->SetVec3("colorVal", color);

        m_pCubeVAO->Bind();
        glLineWidth(2.5f);
        glDrawArrays(GL_LINES, 0, 24);
        glLineWidth(1.0f);
        m_pCubeVAO->Unbind();
    }
    static void DrawLine(const Core::Vec3& objVecStart,
                         const Core::Vec3& objVecEnd,
                         const Core::Vec3& color,
                         const Core::Mat4& viewProjMatrix) {
        if (!m_pPrimitiveShader)
            return;
        m_pPrimitiveShader->Use();

        float fLineVertices[] = {
            objVecStart.x, objVecStart.y, objVecStart.z, objVecEnd.x, objVecEnd.y, objVecEnd.z};

        m_pLineVBO->Bind();
        glBufferData(GL_ARRAY_BUFFER, sizeof(fLineVertices), fLineVertices, GL_DYNAMIC_DRAW);
        m_pLineVBO->Unbind();
        Core::Mat4 objModel;
        Core::Mat4 ObjMVP = viewProjMatrix * objModel;
        m_pPrimitiveShader->SetMat4("uViewProjection", ObjMVP);
        m_pPrimitiveShader->SetVec3("colorVal", color);

        m_pLineVAO->Bind();
        glLineWidth(2.0f);
        glDrawArrays(GL_LINES, 0, 2);
        glLineWidth(1.0f);
        m_pLineVAO->Unbind();
    }

private:
    inline static Shader* m_pPrimitiveShader = nullptr;
    inline static Renderer::VertexArray* m_pCubeVAO = nullptr;
    inline static Renderer::VertexBuffer* m_pCubeVBO = nullptr;

    inline static Renderer::VertexArray* m_pLineVAO = nullptr;
    inline static Renderer::VertexBuffer* m_pLineVBO = nullptr;
};
}  // namespace Renderer