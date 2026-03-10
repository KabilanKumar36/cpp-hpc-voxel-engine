/**
 * @file Application.h
 * @brief Defines the Application class handling the main application state and ImGui UI rendering.
 */

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
 * @brief Manages the application lifecycle, configuration state, and UI integration (ImGui).
 * * This class acts as the central hub for user interface rendering and global
 * application settings such as threading limits, rendering modes, and physics steps.
 */
class Application {
public:
    /**
     * @brief Constructs the application and associates it with a window.
     * @param pWindow Pointer to the active GLFW window context.
     */
    Application(GLFWwindow* pWindow);
    ~Application() = default;

    /**
     * @brief Handles the UI toggle input (typically the Grave Accent / Tilde key).
     * * Switches the application between FPS mode (cursor captured and disabled)
     * and UI mode (cursor freed for ImGui interaction).
     */
    void HandleUIToggle();

    /**
     * @brief Starts a new ImGui frame context.
     * @note Must be called before issuing any ImGui rendering commands in the main loop.
     */
    void BeginImGUIFrame();

    /**
     * @brief Ends the current ImGui frame and dispatches draw data to OpenGL.
     */
    void EndImGUIFrame();

    /**
     * @brief Initializes the ImGui context and configures the GLFW/OpenGL3 backends.
     */
    void InitImGUI();

    /**
     * @brief Shuts down ImGui, releasing all allocated backend and context resources.
     */
    void ShutDownImGUI();

    /**
     * @brief Renders the primary metrics and debugging UI panel.
     * @param inputHandler Reference to the system input handler for displaying control states.
     * @param objChunkManager Reference to the ChunkManager for displaying chunk/threading stats.
     * @param objRayHit Reference to the current RayHit data for block targeting information.
     */
    void RenderMetricsUI(App::InputHandler& inputHandler,
                         const ChunkManager& objChunkManager,
                         const RayHit& objRayHit);

    /**
     * @brief Renders the Help/Controls overlay window.
     */
    void RenderHelpUI();

    // ------------------------------------------------------------------------
    // Public Application State & Configuration Variables
    // ------------------------------------------------------------------------

    float m_fFlySpeed = 200.0f;
    float m_fAccumulator = 20.0f;

    int m_iPhysicsSteps = 0;
    int m_iMainThreads = 1;
    int m_iThermalThreads = 4;
    int m_iActiveThreads = 4;
    int m_iMaxRenderingThreads = 8;

    bool m_bShowMetricsPanel = true;
    bool m_bShowHelpWindow = true;
    bool m_bWireframeMode = false;
    bool m_bHardwareCulling = true;
    bool m_bEnableNeighborCulling = true;
    bool m_bFrustumCulling = true;
    bool m_bFlyMode = false;
    bool m_bEnableVsycn = false;
    bool m_bEnableSIMD = true;

private:
    GLFWwindow* m_pWindow;
};