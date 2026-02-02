#pragma once

#include "../core/camera.h"
#include "../world/Chunk.h"
#include "GLFW/glfw3.h"
#include "glad/glad.h"

namespace App {
class InputHandler {
public:
    static void Init(GLFWwindow* pWindow);
    static void ProcessInput(GLFWwindow* pWindow, float fDeltaTime);
    static void ScrollCallback(GLFWwindow* pWindow, double dXOffset, double dYOffset);
    static void MouseCallback(GLFWwindow* pWindow, double xPosIn, double yPosIn);
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void UpdateTitleInfo(GLFWwindow* pWindow);
    static void processFirePreviewAndFire(GLFWwindow* pWindow,
                                          std::vector<Chunk>& chunks,
                                          const Core::Mat4& viewProjection);
    static float GetLastX() { return m_fLastX; }
    static void SetLastX(float fValue) { m_fLastX = fValue; }
    static float GetLastY() { return m_fLastY; }
    static void SetLastY(float fValue) { m_fLastY = fValue; }
    static float GetOrthoSize() { return m_fOrthoSize; }
    static void SetOrthoSize(float fValue) { m_fOrthoSize = fValue; }

    static bool IsFirstMouse() { return m_bFirstMouse; }
    static void SetFirstMouse(bool bValue) { m_bFirstMouse = bValue; }
    static bool IsEnableFaceCulling() { return m_bEnableFaceCulling; }
    static void SetEnableFaceCulling(bool bValue) { m_bEnableFaceCulling = bValue; }
    static bool IsPerspective() { return m_bPerspective; }
    static void SetPerspective(bool bValue) { m_bPerspective = bValue; }
    static bool IsLMBClickedFirstTime() { return m_bLMBClickedFirstTime; }
    static void SetLMBClickedFirstTime(bool bValue) { m_bLMBClickedFirstTime = bValue; }
    static bool IsFKeyPressedLastTime() { return m_bFKeyPressedLastTime; }
    static void SetFKeyPressedLastTime(bool bValue) { m_bFKeyPressedLastTime = bValue; }
    static bool IsPKeyPressedLastTime() { return m_bPKeyPressedLastTime; }
    static void SetPKeyPressedLastTime(bool bValue) { m_bPKeyPressedLastTime = bValue; }
    static Core::Camera& GetCamera() { return m_objCamera; }

    static unsigned int GetScreenWidth() { return SCREEN_WIDTH; }
    static unsigned int GetScreenHeight() { return SCREEN_HEIGHT; }

    static int GetFrameCount() { return m_iFrameCount; }
    static void AddFrameCount() { m_iFrameCount++; }
    static void ResetCounters();
    static float GetTime() { return m_fTimer; }
    static void SetDeltaTime(float fDelTime);
    static Core::Mat4 GetViewProjectionMatrix();

private:
    static Core::Camera m_objCamera;
    static Core::Vec3 m_objCameraPos;
    static float m_fLastX;
    static float m_fLastY;
    static float m_fOrthoSize;
    static float m_fTimer;
    static float m_fDeltaTime;
    static int m_iFrameCount;
    static const unsigned int SCREEN_WIDTH;
    static const unsigned int SCREEN_HEIGHT;
    static bool m_bFirstMouse;
    static bool m_bEnableFaceCulling;
    static bool m_bPerspective;
    static bool m_bLMBClickedFirstTime;
    static bool m_bFKeyPressedLastTime;
    static bool m_bPKeyPressedLastTime;
};
}  // namespace App