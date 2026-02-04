#pragma once

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "core/MathUtils.h"

class InputManager {
public:
    static InputManager &GetInstance() {
        static InputManager objInstance;
        return objInstance;
    }

    void Init(GLFWwindow *pWindow);
    void Update();

    bool IsKeyPressed(int iKey) const;
    bool IsKeyJustPressed(int iKey) const;

    static void FrameBufferSizeCallback(GLFWwindow *window, int width, int height);
    bool IsMouseButtonPressed(int iKey) const;
    Core::Vec3 GetMousePosition() const { return m_objMousePosition; }
    Core::Vec3 GetMouseDelta() const { return m_objMouseDelta; }
    double GetScroll() const { return m_dScrollY; }

private:
    InputManager() = default;

    static void KeyCallback(GLFWwindow *pWindow,
                            int iKey,
                            [[maybe_unused]] int iScanCode,
                            int iAction,
                            [[maybe_unused]] int iMods);
    static void MouseButtonCallback(GLFWwindow *pWindow,
                                    int iKey,
                                    int iAction,
                                    [[maybe_unused]] int iMods);
    static void MousePosCallback(GLFWwindow *pWindow, double dXPos, double dYPos);
    static void ScrollCallback(GLFWwindow *pWindow, double dXOffset, double dYOffset);

    bool m_bKeys[1024] = {false};
    bool m_bPrevKeys[1024] = {false};

    bool m_bButtons[8] = {false};

    Core::Vec3 m_objMousePosition;
    Core::Vec3 m_objMousePrevPosition;
    Core::Vec3 m_objMouseDelta;
    double m_dScrollY = 0.0;
    bool m_bFirstMouse = false;
};