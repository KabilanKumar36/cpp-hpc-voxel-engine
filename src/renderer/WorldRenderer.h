#pragma once

#include "../core/Matrix.h"
#include "../physics/PhysicsSystem.h"
#include "../world/Chunk.h"
#include "PrimitiveRenderer.h"
#include "Shader.h"
#include "glad/glad.h"

namespace Renderer {
class WorldRenderer {
public:
    static inline void DrawAxes(const Core::Mat4 &objViewProjection, float fLength = 50.0f) {
        glDisable(GL_DEPTH_TEST);
        PrimitiveRenderer::DrawLine(Core::Vec3(0.0f, 0.0f, 0.0f),
                                    Core::Vec3(fLength, 0.0f, 0.0f),
                                    Core::Vec3(1.0f, 0.0f, 0.0f),
                                    objViewProjection);
        PrimitiveRenderer::DrawLine(Core::Vec3(0.0f, 0.0f, 0.0f),
                                    Core::Vec3(0.0f, fLength, 0.0f),
                                    Core::Vec3(0.0f, 1.0f, 0.0f),
                                    objViewProjection);
        PrimitiveRenderer::DrawLine(Core::Vec3(0.0f, 0.0f, 0.0f),
                                    Core::Vec3(0.0f, 0.0f, fLength),
                                    Core::Vec3(0.0f, 0.0f, 1.0f),
                                    objViewProjection);
        glEnable(GL_DEPTH_TEST);
    }

    static inline void DrawChunks(const std::vector<Chunk> &chunks,
                                  Renderer::Shader &shader,
                                  const Core::Mat4 &objViewProjection) {
        shader.use();
        shader.setMat4("uViewProjection", objViewProjection);
        for (auto &chunk : chunks) {
            chunk.Render();
        }
    }
};
}  // namespace Renderer