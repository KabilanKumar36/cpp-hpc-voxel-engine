#pragma once

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"

#include "InputHandler.h"
// ********************************************************************
struct GLFWwindow;
struct RayHit;
class ChunkManager;
// ********************************************************************

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

    /**
     * @brief Handles the UI Rendering.
     */
    void RenderMetricsUI(App::InputHandler& inputHandler,
                         const ChunkManager& objChunkManager,
                         const RayHit& objRayHit);

    /**
     * @brief Handles the Help Rendering.
     */
    void RenderHelpUI();

    bool m_bShowMetricsPanel = true;
    bool m_bShowHelpWindow = true;

    bool m_bWireframeMode = false;
    bool m_bHardwareCulling = true;
    bool m_bEnableNeighborCulling = true;
    bool m_bFrustumCulling = true;
    bool m_bFlyMode = false;
    int m_iActiveThreads = 4;
    int m_iMaxHardwareThreads = 8;
    float m_fFlySpeed = 20.0f;

private:
    GLFWwindow* m_pWindow;
};