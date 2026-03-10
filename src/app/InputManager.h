/**
 * @file InputManager.h
 * @brief Defines the InputManager singleton for global keyboard and mouse state polling.
 */

#pragma once

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "../core/MathUtils.h"

/**
 * @class InputManager
 * @brief Singleton class that handles global input state.
 */
class InputManager {
public:
    /**
     * @brief Retrieves the singleton instance.
     */
    static InputManager &GetInstance() {
        static InputManager objInstance;
        return objInstance;
    }

    /**
     * @brief Initializes input callbacks for the specified window.
     * @param pWindow The GLFW window handle.
     */
    void Init(GLFWwindow *pWindow);

    /**
     * @brief Updates internal state (previous keys, mouse delta) at the start of a frame.
     * @note Must be called BEFORE glfwPollEvents() for IsKeyJustPressed to work correctly.
     */
    void Update();

    bool IsKeyPressed(int iKey) const;
    bool IsKeyJustPressed(int iKey) const;
    bool IsMouseButtonPressed(int iButton) const;

    Core::Vec3 GetMousePosition() const { return m_objMousePosition; }
    Core::Vec3 GetMouseDelta() const { return m_objMouseDelta; }
    double GetScroll() const { return m_dScrollY; }

    static void FrameBufferSizeCallback(GLFWwindow *window, int width, int height);

private:
    InputManager() = default;

    static void keyCallback(GLFWwindow *pWindow, int iKey, int iScanCode, int iAction, int iMods);
    static void mouseButtonCallback(GLFWwindow *pWindow, int iButton, int iAction, int iMods);
    static void mousePosCallback(GLFWwindow *pWindow, double dXPos, double dYPos);
    static void scrollCallback(GLFWwindow *pWindow, double dXOffset, double dYOffset);

    bool m_bKeys[1024] = {false};
    bool m_bPrevKeys[1024] = {false};

    bool m_bButtons[8] = {false};

    Core::Vec3 m_objMousePosition;
    Core::Vec3 m_objMousePrevPosition;
    Core::Vec3 m_objMouseDelta;
    double m_dScrollY = 0.0;
    bool m_bFirstMouse = false;
};