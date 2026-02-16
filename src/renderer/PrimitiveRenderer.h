#pragma once

#include "../core/MathUtils.h"
#include "../core/Matrix.h"
#include "Buffer.h"
#include "Shader.h"
#include "VertexArray.h"

namespace Renderer {

/**
 * @class PrimitiveRenderer
 * @brief A static helper for drawing immediate-mode debug shapes (Lines, Cubes).
 * Useful for visualizing physics colliders, raycasts, and axes.
 */
class PrimitiveRenderer {
public:
    /**
     * @brief Initializes the debug shader and static geometry buffers.
     * Must be called after OpenGL context is created.
     */
    static void Init();

    /**
     * @brief Cleans up shaders and buffers.
     */
    static void Shutdown();

    /**
     * @brief Draws a wireframe cube.
     * @param position Center of the cube.
     * @param size Half-extents (Radius) of the cube.
     * @param color RGB color.
     * @param viewProjMatrix Camera View * Projection matrix.
     */
    static void DrawCube(const Core::Vec3& position,
                         const Core::Vec3& size,
                         const Core::Vec3& color,
                         const Core::Mat4& viewProjMatrix);

    /**
     * @brief Draws a line between two points.
     */
    static void DrawLine(const Core::Vec3& objVecStart,
                         const Core::Vec3& objVecEnd,
                         const Core::Vec3& color,
                         const Core::Mat4& viewProjMatrix);

private:
    static Shader* m_pPrimitiveShader;

    // Cube Resources
    static VertexArray* m_pCubeVAO;
    static VertexBuffer* m_pCubeVBO;

    // Line Resources
    static VertexArray* m_pLineVAO;
    static VertexBuffer* m_pLineVBO;
};
}  // namespace Renderer