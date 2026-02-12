#pragma once

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"

class Application {
public:
    Application(GLFWwindow* m_pWindow);
    ~Application() = default;
    void HandleUIToggle();
    void BeginImGUIFrame();
    void EndImGUIFrame();
    void InitImGUI();
    void ShutDownImGUI();

    bool m_bIsUIActive = false;
    bool m_bShowDebugPanel = true;

private:
    GLFWwindow* m_pWindow;
};