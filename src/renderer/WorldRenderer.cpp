#include "WorldRenderer.h"
#include "../world/Chunk.h"
#include "Frustum.h"
#include "PrimitiveRenderer.h"

namespace Renderer {

// ********************************************************************
void WorldRenderer::DrawAxes(const Core::Mat4 &objViewProjection, float fLength) {
    glDisable(GL_DEPTH_TEST);  // Draw on top of everything

    // X-Axis (Red)
    PrimitiveRenderer::DrawLine(Core::Vec3(0.0f, 0.0f, 0.0f),
                                Core::Vec3(fLength, 0.0f, 0.0f),
                                Core::Vec3(1.0f, 0.0f, 0.0f),
                                objViewProjection);
    // Y-Axis (Green)
    PrimitiveRenderer::DrawLine(Core::Vec3(0.0f, 0.0f, 0.0f),
                                Core::Vec3(0.0f, fLength, 0.0f),
                                Core::Vec3(0.0f, 1.0f, 0.0f),
                                objViewProjection);
    // Z-Axis (Blue)
    PrimitiveRenderer::DrawLine(Core::Vec3(0.0f, 0.0f, 0.0f),
                                Core::Vec3(0.0f, 0.0f, fLength),
                                Core::Vec3(0.0f, 0.0f, 1.0f),
                                objViewProjection);

    glEnable(GL_DEPTH_TEST);
}

// ********************************************************************
void WorldRenderer::DrawChunks(ChunkManager &objChunkManager,
                               Renderer::Shader &shader,
                               const Core::Mat4 &objViewProjection,
                               bool bEnableFrustumCulling) {
    shader.Use();
    shader.SetMat4("uViewProjection", objViewProjection);

    Frustum objFrustum;
    if (bEnableFrustumCulling) {
        objFrustum.Update(objViewProjection);
    }

    // Iterate over chunks
    for (const auto &[Coords, objChunk] : objChunkManager.GetMutableChunks()) {
        // Frustum Culling Check
        if (bEnableFrustumCulling) {
            // GetAABB() returns the world-space bounding box of the chunk
            if (!objFrustum.IsBoxInVisibleFrustum(objChunk.GetAABB())) {
                continue;  // Skip rendering this chunk
            }
        }

        objChunk.Render();
    }
}

}  // namespace Renderer