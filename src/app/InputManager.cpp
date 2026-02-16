#include "InputManager.h"
#include <cstring>

//*********************************************************************
void InputManager::Init(GLFWwindow *pWindow) {
    if (!pWindow)
        return;

    glfwSetWindowUserPointer(pWindow, this);

    glfwSetKeyCallback(pWindow, keyCallback);
    glfwSetMouseButtonCallback(pWindow, mouseButtonCallback);
    glfwSetCursorPosCallback(pWindow, mousePosCallback);
    glfwSetScrollCallback(pWindow, scrollCallback);
    glfwSetFramebufferSizeCallback(pWindow, FrameBufferSizeCallback);

    m_bFirstMouse = true;
}
//*********************************************************************
void InputManager::FrameBufferSizeCallback([[maybe_unused]] GLFWwindow *window,
                                           int width,
                                           int height) {
    glViewport(0, 0, width, height);
}
//*********************************************************************
void InputManager::Update() {
    // Reset per-frame data
    m_dScrollY = 0.0;

    // Copy current key state to previous state (for JustPressed logic)
    std::memcpy(m_bPrevKeys, m_bKeys, 1024 * sizeof(bool));

    // Calculate Mouse Delta
    // Note: This delta calculation assumes Update() is called *before* PollEvents
    // if we want delta from the *previous* frame's movement.
    m_objMouseDelta.x = m_objMousePosition.x - m_objMousePrevPosition.x;
    m_objMouseDelta.y =
        m_objMousePrevPosition.y - m_objMousePosition.y;  // Inverted Y for OpenGL camera
    m_objMousePrevPosition = m_objMousePosition;
}
//*********************************************************************
bool InputManager::IsKeyPressed(int iKey) const {
    if (iKey >= 0 && iKey < 1024)
        return m_bKeys[iKey];
    return false;
}
//*********************************************************************
bool InputManager::IsKeyJustPressed(int iKey) const {
    if (iKey >= 0 && iKey < 1024)
        // Key is down NOW, but was NOT down LAST FRAME
        return m_bKeys[iKey] && !m_bPrevKeys[iKey];
    return false;
}
//*********************************************************************
bool InputManager::IsMouseButtonPressed(int iButton) const {
    if (iButton >= 0 && iButton < 8)
        return m_bButtons[iButton];
    return false;
}
//*********************************************************************
void InputManager::keyCallback([[maybe_unused]] GLFWwindow *pWindow,
                               int iKey,
                               [[maybe_unused]] int iScanCode,
                               int iAction,
                               [[maybe_unused]] int iMods) {
    InputManager &inputs = InputManager::GetInstance();
    if (iKey >= 0 && iKey < 1024) {
        if (iAction == GLFW_PRESS)
            inputs.m_bKeys[iKey] = true;
        else if (iAction == GLFW_RELEASE)
            inputs.m_bKeys[iKey] = false;
    }
}
//*********************************************************************
void InputManager::mouseButtonCallback([[maybe_unused]] GLFWwindow *pWindow,
                                       int iButton,
                                       int iAction,
                                       [[maybe_unused]] int iMods) {
    InputManager &inputs = InputManager::GetInstance();
    if (iButton >= 0 && iButton < 8) {
        if (iAction == GLFW_PRESS)
            inputs.m_bButtons[iButton] = true;
        else if (iAction == GLFW_RELEASE)
            inputs.m_bButtons[iButton] = false;
    }
}
//*********************************************************************
void InputManager::mousePosCallback([[maybe_unused]] GLFWwindow *pWindow,
                                    double dXPos,
                                    double dYPos) {
    InputManager &inputs = InputManager::GetInstance();
    if (inputs.m_bFirstMouse) {
        inputs.m_objMousePrevPosition = {
            static_cast<float>(dXPos), static_cast<float>(dYPos), 0.0f};
        inputs.m_bFirstMouse = false;
    }
    inputs.m_objMousePosition = {static_cast<float>(dXPos), static_cast<float>(dYPos), 0.0f};
}
//*********************************************************************
void InputManager::scrollCallback([[maybe_unused]] GLFWwindow *pWindow,
                                  [[maybe_unused]] double dXOffset,
                                  double dYOffset) {
    InputManager &inputs = InputManager::GetInstance();
    inputs.m_dScrollY = static_cast<float>(dYOffset);
}
//*********************************************************************