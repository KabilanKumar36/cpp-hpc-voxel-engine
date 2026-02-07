#pragma once
#include <memory>

// clang-format off
#include "glad/glad.h"
#include "GLFW/glfw3.h"
// clang-format on

#include "../core/camera.h"
#include "../world/Chunk.h"
#include "../world/Player.h"

namespace App {
class InputHandler {
public:
    InputHandler(const Core::Vec3& ObjStartPos);
    void ProcessInput(GLFWwindow* pWindow, float fDeltaTime);
    void UpdatePlayerPhysics(float fDeltaTime, const ChunkManager& objChunkManager);

    void UpdateTitleInfo(GLFWwindow* pWindow) const;
    void processFirePreviewAndFire(ChunkManager& objChunkManager, const Core::Mat4& viewProjection);
    float GetOrthoSize() const { return m_fOrthoSize; }
    void SetOrthoSize(float fValue) { m_fOrthoSize = fValue; }

    bool IsEnableFaceCulling() const { return m_bEnableFaceCulling; }
    void SetEnableFaceCulling(bool bValue) { m_bEnableFaceCulling = bValue; }
    bool IsPerspective() const { return m_bPerspective; }
    void SetPerspective(bool bValue) { m_bPerspective = bValue; }
    bool IsLMBClickedFirstTime() const { return m_bLMBClickedFirstTime; }
    void SetLMBClickedFirstTime(bool bValue) { m_bLMBClickedFirstTime = bValue; }

    Core::Camera& GetCamera() { return m_pPlayer->GetCamera(); }
    Player* GetPlayer() { return m_pPlayer.get(); }

    unsigned int GetScreenWidth() const { return SCREEN_WIDTH; }
    unsigned int GetScreenHeight() const { return SCREEN_HEIGHT; }

    int GetFrameCount() const { return m_iFrameCount; }
    void AddFrameCount() { m_iFrameCount++; }
    void ResetCounters();
    float GetTime() const { return m_fTimer; }
    void SetDeltaTime(float fDelTime);
    Core::Mat4 GetViewProjectionMatrix();

private:
    Core::Vec3 m_objCameraPos;
    std::unique_ptr<Player> m_pPlayer;

    float m_fOrthoSize = 10.0f;
    float m_fTimer = 0.0f;
    float m_fDeltaTime = 0.0f;

    int m_iFrameCount = 0;
    const unsigned int SCREEN_WIDTH = 1280;
    const unsigned int SCREEN_HEIGHT = 720;

    bool m_bEnableFaceCulling = false;
    bool m_bPerspective = true;
    bool m_bLMBClickedFirstTime = false;
};
}  // namespace App