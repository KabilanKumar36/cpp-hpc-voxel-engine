#pragma once

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"

// Forward declaration
struct GLFWwindow;

/**
 * @class Application
 * @brief Manages the application lifecycle and UI integration (ImGui).
 */
class Application {
public:
    /**
     * @brief Constructs the application with a window handle.
     * @param pWindow Pointer to the GLFW window.
     */
    Application(GLFWwindow* pWindow);
    ~Application() = default;

    /**
     * @brief Handles the UI toggle input (Grave Accent / Tilde key).
     * Switches between FPS mode (cursor disabled) and UI mode (cursor enabled).
     */
    void HandleUIToggle();

    /**
     * @brief Starts a new ImGui frame. Must be called before any UI rendering.
     */
    void BeginImGUIFrame();

    /**
     * @brief Ends the ImGui frame and renders draw data.
     */
    void EndImGUIFrame();

    /**
     * @brief Initializes ImGui context and backends.
     */
    void InitImGUI();

    /**
     * @brief Shuts down ImGui and releases resources.
     */
    void ShutDownImGUI();

    bool m_bIsUIActive = false;     ///< True if the UI overlay is currently interactive.
    bool m_bShowDebugPanel = true;  ///< True if the debug panel should be drawn.

private:
    GLFWwindow* m_pWindow;
};