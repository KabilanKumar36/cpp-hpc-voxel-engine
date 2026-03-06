#pragma once
#include <memory>

// clang-format off
#include "glad/glad.h"
#include "GLFW/glfw3.h"
// clang-format on

#include "../core/camera.h"
#include "../physics/PhysicsSystem.h"
#include "../world/Chunk.h"
#include "../world/ChunkManager.h"
#include "../world/Player.h"

namespace App {
class InputHandler {
public:
    InputHandler(const Core::Vec3& ObjStartPos);
    void ProcessInput(GLFWwindow* pWindow, ChunkManager& objChunkManager, float fDeltaTime);
    void UpdatePlayerPhysics(float fDeltaTime, const ChunkManager& objChunkManager);

    // Raycasts into the world to preview/edit blocks. Returns hit result for logic processing.
    RayHit ProcessFirePreviewAndFire(ChunkManager& objChunkManager,
                                     const Core::Mat4& viewProjection);

    float GetOrthoSize() const { return m_fOrthoSize; }
    void SetOrthoSize(float fValue) { m_fOrthoSize = fValue; }

    bool IsSIMDEnabled() const { return m_bEnableSIMD; }
    void SetEnableSIMD(bool bValue) { m_bEnableSIMD = bValue; }

    bool IsNeighborCullingEnabled() const { return m_bNeighborCullingEnabled; }
    void SetNeighborCullingEnable(bool bValue) { m_bNeighborCullingEnabled = bValue; }

    bool IsFrustumCullingEnabled() const { return m_bFrustumCullingEnabled; }
    void SetFrustumCullingEnable(bool bValue) { m_bFrustumCullingEnabled = bValue; }

    int GetActiveThreads() const { return m_iActiveThreads; }
    void SetActiveThreads(int iCt) { m_iActiveThreads = iCt; }

    bool IsPerspective() const { return m_bPerspective; }
    void SetPerspective(bool bValue) { m_bPerspective = bValue; }
    bool IsLMBClickedFirstTime() const { return m_bLMBClickedFirstTime; }
    void SetLMBClickedFirstTime(bool bValue) { m_bLMBClickedFirstTime = bValue; }

    Core::Camera& GetCamera() { return m_pPlayer->GetCamera(); }
    Player* GetPlayer() { return m_pPlayer.get(); }
    void SetMovementSpeed(float fMoveSpeed) {
        if (m_pPlayer)
            m_pPlayer->SetMovementSpeed(fMoveSpeed);
    }
    void SetFlyMode(bool bFlyMode) { m_bFlyMode = bFlyMode; }
    void SetScreenWidth(int iWidth, int iHeight) {
        m_iScreenWidth = iWidth;
        m_iScreenHeight = iHeight;
    }
    unsigned int GetScreenWidth() const { return m_iScreenWidth; }
    unsigned int GetScreenHeight() const { return m_iScreenHeight; }

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
    int m_iActiveThreads = 0;
    int m_iScreenWidth = 1920;
    int m_iScreenHeight = 1080;

    bool m_bEnableSIMD = true;
    bool m_bNeighborCullingEnabled = true;
    bool m_bFrustumCullingEnabled = true;
    bool m_bPerspective = true;
    bool m_bEscClickedFirstTime = false;
    bool m_bLMBClickedFirstTime = false;
    bool m_bRMBClickedFirstTime = false;
    bool m_bMMBClickedFirstTime = false;
    bool m_bFlyMode = false;
};
}  // namespace App