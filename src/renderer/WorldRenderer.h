#pragma once

#include "../core/Matrix.h"
#include "../world/ChunkManager.h"
#include "Shader.h"

namespace Renderer {

/**
 * @class WorldRenderer
 * @brief High-level renderer for the Voxel World and Global Axes.
 */
class WorldRenderer {
public:
    /**
     * @brief Draws the RGB coordinate axes at the origin.
     * @param objViewProjection Camera VP Matrix.
     * @param fLength Length of the axis lines.
     */
    static void DrawAxes(const Core::Mat4 &objViewProjection, float fLength = 50.0f);

    /**
     * @brief Renders the voxel world chunks.
     * @param objChunkManager The world manager containing chunks.
     * @param shader The main voxel shader.
     * @param objViewProjection Camera VP Matrix.
     * @param bEnableFrustumCulling If true, skips chunks outside the camera view.
     */
    static void DrawChunks(ChunkManager &objChunkManager,
                           Renderer::Shader &shader,
                           const Core::Mat4 &objViewProjection,
                           bool bEnableFrustumCulling = false);
};
}  // namespace Renderer