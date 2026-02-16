#include "PrimitiveRenderer.h"
#include <vector>

namespace Renderer {

// Define static members
Shader* PrimitiveRenderer::m_pPrimitiveShader = nullptr;
VertexArray* PrimitiveRenderer::m_pCubeVAO = nullptr;
VertexBuffer* PrimitiveRenderer::m_pCubeVBO = nullptr;
VertexArray* PrimitiveRenderer::m_pLineVAO = nullptr;
VertexBuffer* PrimitiveRenderer::m_pLineVBO = nullptr;

// ********************************************************************
void PrimitiveRenderer::Init() {
    // 1. Compile Debug Shader
    m_pPrimitiveShader = new Shader("assets/shaders/vertex_SimpleDebug.glsl",
                                    "assets/shaders/fragment_SimpleDebug.glsl");

    // 2. Setup Cube (Line List)
    // A cube has 12 edges, so we need 24 vertices for GL_LINES
    std::vector<float> cubeVertices = {
        0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1,
        1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0,  // Bottom Face
        0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1,
        1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0,  // Top Face
        0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0,
        1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1  // Connecting Pillars
    };

    m_pCubeVAO = new VertexArray();
    m_pCubeVBO = new VertexBuffer(cubeVertices.data(),
                                  static_cast<unsigned int>(cubeVertices.size()) * sizeof(float));

    m_pCubeVAO->LinkAttribute(*m_pCubeVBO, 0, 3, 0, 0);
    m_pCubeVAO->Unbind();

    // 3. Setup Dynamic Line
    float fLineVertices[] = {0, 0, 0, 0, 0, 0};
    m_pLineVBO = new VertexBuffer(fLineVertices, sizeof(fLineVertices));
    m_pLineVAO = new VertexArray();
    m_pLineVAO->LinkAttribute(*m_pLineVBO, 0, 3, 0, 0);
}

// ********************************************************************
void PrimitiveRenderer::Shutdown() {
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

// ********************************************************************
void PrimitiveRenderer::DrawCube(const Core::Vec3& position,
                                 const Core::Vec3& size,
                                 const Core::Vec3& color,
                                 const Core::Mat4& viewProjMatrix) {
    if (!m_pPrimitiveShader)
        return;

    m_pPrimitiveShader->Use();

    // Model Matrix: Translate then Scale
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

// ********************************************************************
void PrimitiveRenderer::DrawLine(const Core::Vec3& objVecStart,
                                 const Core::Vec3& objVecEnd,
                                 const Core::Vec3& color,
                                 const Core::Mat4& viewProjMatrix) {
    if (!m_pPrimitiveShader)
        return;

    m_pPrimitiveShader->Use();

    // Update VBO data dynamically
    float fLineVertices[] = {
        objVecStart.x, objVecStart.y, objVecStart.z, objVecEnd.x, objVecEnd.y, objVecEnd.z};

    m_pLineVBO->Bind();
    glBufferData(GL_ARRAY_BUFFER, sizeof(fLineVertices), fLineVertices, GL_DYNAMIC_DRAW);
    m_pLineVBO->Unbind();

    // Identity Model Matrix for lines (world space coords)
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

}  // namespace Renderer