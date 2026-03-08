#include "Application.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "physics/PhysicsSystem.h"
#include "world/ChunkManager.h"

//*********************************************************************
Application::Application(GLFWwindow* pWindow) : m_pWindow(pWindow) {}
//*********************************************************************
void Application::InitImGUI() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& IO = ImGui::GetIO();
    IO.FontGlobalScale = 1.5f;

    IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_pWindow, true);
    ImGui_ImplOpenGL3_Init("#version 450");

    m_iActiveThreads = m_iMaxRenderingThreads;
}
//*********************************************************************
void Application::ShutDownImGUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
//*********************************************************************
void Application::BeginImGUIFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
//*********************************************************************
void Application::RenderMetricsUI(App::InputHandler& inputHandler,
                                  const ChunkManager& objChunkManager,
                                  const RayHit& objRayHit) {
    if (!m_bShowMetricsPanel)
        return;
    ImGuiViewport* pViewPort = ImGui::GetMainViewport();
    ImVec2 objWorkPos = pViewPort->WorkPos;
    ImVec2 objWorkSize = pViewPort->WorkSize;
    float fPadding = 15.0f;

    ImGuiWindowFlags objWindowFlags =
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
    if (!m_bShowHelpWindow) {
        objWindowFlags |= ImGuiWindowFlags_NoInputs;
    }

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.8f));

    // =====================================================================
    // WINDOW 0: Wireframe & Fly Mode (Top Left Corner)
    // =====================================================================
    ImGui::SetNextWindowPos(ImVec2(objWorkPos.x + fPadding, objWorkPos.y + fPadding),
                            ImGuiCond_Always,
                            ImVec2(0.0f, 0.0f));
    ImGui::Begin("Controls", nullptr, objWindowFlags);
    if (ImGui::Checkbox("Wireframe", &m_bWireframeMode)) {
        glPolygonMode(GL_FRONT_AND_BACK, m_bWireframeMode ? GL_LINE : GL_FILL);
    }
    if (ImGui::Checkbox("Fly Mode", &m_bFlyMode)) {
        inputHandler.SetFlyMode(m_bFlyMode);
        if (!m_bFlyMode)
            m_fFlySpeed = 5.0f;
        inputHandler.SetMovementSpeed(m_fFlySpeed);
    }
    ImGui::BeginDisabled(!m_bFlyMode);
    if (ImGui::SliderFloat("Fly Speed", &m_fFlySpeed, 1.0f, 200.0f)) {
        inputHandler.SetMovementSpeed(m_fFlySpeed);
    }
    ImGui::EndDisabled();

    Core::Vec3 objCurrCameraPos = inputHandler.GetCamera().GetCameraPosition();
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "Position");
    ImGui::Text(
        "X: %0.3f, Y: %0.3f, Z: %0.3f", objCurrCameraPos.x, objCurrCameraPos.y, objCurrCameraPos.z);

    ImGui::End();

    // =====================================================================
    // WINDOW 1: Performance & Optimizations (Bottom Left Corner)
    // =====================================================================
    ImGui::SetNextWindowPos(
        ImVec2(objWorkPos.x + fPadding, objWorkPos.y + objWorkSize.y - fPadding),
        ImGuiCond_Always,
        ImVec2(0.0f, 1.0f));
    ImGui::Begin("Performance & Optimizations", nullptr, objWindowFlags);
    if (ImGui::CollapsingHeader("Optimizations", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Checkbox("GPU Back Face Culling", &m_bHardwareCulling)) {
            if (m_bHardwareCulling) {
                glEnable(GL_CULL_FACE);
            } else {
                glDisable(GL_CULL_FACE);
            }
        }
        if (ImGui::Checkbox("Neighbor Face Culling", &m_bEnableNeighborCulling)) {
            inputHandler.SetNeighborCullingEnable(m_bEnableNeighborCulling);
        }
        if (ImGui::Checkbox("Frustrum Culling", &m_bFrustumCulling)) {
            inputHandler.SetFrustumCullingEnable(m_bFrustumCulling);
        }
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.2f, 1.0f));
        if (ImGui::Checkbox("Enable SIMD", &m_bEnableSIMD)) {
            inputHandler.SetEnableSIMD(m_bEnableSIMD);
        }
        ImGui::PopStyleColor();
    }

    if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("FPS: %0.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Frame Time: %0.3f ms", 1000.0f / ImGui::GetIO().Framerate);
        ImGui::Text("Chunks Loaded: %zu", objChunkManager.GetChunks().size());
    }
    ImGui::End();

    // =====================================================================
    // WINDOW 2: System Architecture & Physics (Top Right Corner)
    // =====================================================================
    ImGui::SetNextWindowPos(
        ImVec2(objWorkPos.x + objWorkSize.x - fPadding, objWorkPos.y + fPadding),
        ImGuiCond_Always,
        ImVec2(1.0f, 0.0f));
    ImGui::Begin("System & Physics", nullptr, objWindowFlags);
    if (ImGui::CollapsingHeader("Threading Subsystem", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Main Thread Count: %d", m_iMainThreads);
        ImGui::Text("Thermal Threads: %d", m_iThermalThreads);
        ImGui::TextColored(ImVec4(0, 1, 1, 1), "Rendering Threads");
        ImGui::Text("Max. Avl. Render Threads: %d", m_iMaxRenderingThreads);
        if (ImGui::SliderInt("Count", &m_iActiveThreads, 0, m_iMaxRenderingThreads)) {
            inputHandler.SetActiveThreads(m_iActiveThreads);
        }
        if (m_iActiveThreads == 0) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "WARNING: Running on Main Thread (Sync Mode)");
        }
    }
    if (ImGui::CollapsingHeader("Mesh Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextColored(ImVec4(0.8f, 0.2f, 1.0f, 1), "Mesh Stats");
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1), "Generated By CPU");
        ImGui::Text("Vertices: %zu", objChunkManager.GetGeneratedVertCount());
        ImGui::Text("Triangles: %zu", objChunkManager.GetGeneratedTriaCount());

        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.8f, 1), "Uploaded To GPU (Within Frustum)");
        ImGui::Text("Vertices: %zu", objChunkManager.GetUploadedVertCount());
        ImGui::Text("Triangles: %zu", objChunkManager.GetUploadedTriaCount());
    }
    if (ImGui::CollapsingHeader("Physics Engine", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Checkbox("Enable V Sync", &m_bEnableVsycn)) {
            if (m_bEnableVsycn)
                glfwSwapInterval(1);
            else
                glfwSwapInterval(0);
        }
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Decouple Render and Physics");
        ImGui::Text("Physics Steps/Frame: %d", m_iPhysicsSteps);
        ImGui::Text("Accumulator Remainder: %0.2f", m_fAccumulator * 1000.0f);
    }
    ImGui::End();

    // =====================================================================
    // WINDOW 3: Player Interaction (Bottom Right Corner)
    // =====================================================================
    ImGui::SetNextWindowPos(
        ImVec2(objWorkPos.x + objWorkSize.x - fPadding, objWorkPos.y + objWorkSize.y - fPadding),
        ImGuiCond_Always,
        ImVec2(1.0f, 1.0f));
    if (ImGui::GetIO().KeyCtrl) {
        ImGui::Begin("Interaction Data", nullptr, objWindowFlags);
        if (objRayHit.m_bHit) {
            ImGui::Text("Target: Block (%d, %d, %d)",
                        objRayHit.m_iBlocKX,
                        objRayHit.m_iBlocKY,
                        objRayHit.m_iBlocKZ);
            ImGui::Text("Distance: %.2f", objRayHit.m_fDistance);
        }
        ImGui::End();
    }

    ImGui::PopStyleColor();
}
//*********************************************************************
void Application::RenderHelpUI() {
    if (!m_bShowHelpWindow)
        return;
    ImVec2 objCenterPos = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(objCenterPos, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(450, 650), ImGuiCond_FirstUseEver);
    ImGui::Begin("Engine Controls & Help (F1)", &m_bShowHelpWindow);

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Navigation");
    ImGui::BulletText("W A S D: Move Camera/Player");
    ImGui::BulletText("Space Bar: Jump Camera/Player");
    ImGui::BulletText("Mouse: Look Around");
    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Demonstration Guide");
    ImGui::TextWrapped(
        "Open the Developer System Monitor (~)"
        "Try dropping the 'Worker Threads' to 0 to force synchronous main-thread execution"
        "or disable culling to see raw geometry load.");
    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "System Shortcuts");
    ImGui::BulletText("F1: Toogle this Help Menu");
    ImGui::BulletText("Tilde(~): Toggle System Monitor");
    ImGui::BulletText("F: Toggle Frustrum Culling");
    ImGui::BulletText("P: Toggle Ortho/Perspective");
    ImGui::BulletText("R: Reset Camera Direction");
    ImGui::BulletText("Ctrl: Cast Ray on Target and use:");
    ImGui::Text("   LMB: Breaks Target Block");
    ImGui::Text("   RMB: Inserts Block before Target");
    ImGui::Text("   MMB: Injects Heat on Target Block");

    ImGui::Separator();
    ImGui::Text("");
    ImGui::Text("");
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
    if (ImGui::Button("Exit Engine", ImVec2(-1, 0))) {
        glfwSetWindowShouldClose(m_pWindow, true);
    }
    ImGui::PopStyleColor(3);
    ImGui::Text("");

    ImGui::End();
}
//*********************************************************************
void Application::EndImGUIFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
//*********************************************************************
void Application::HandleUIToggle() {
    static bool bKeyWasPressed = false;
    // Toggle UI state on Grave Accent (`~`) press
    if (glfwGetKey(m_pWindow, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS) {
        if (!bKeyWasPressed) {
            bKeyWasPressed = true;
            m_bShowMetricsPanel = !m_bShowMetricsPanel;

            // Switch cursor mode based on UI state
            if (m_bShowMetricsPanel) {
                glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
#ifdef _WIN32
                glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#else
                glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
#endif
            }
        }
    } else {
        bKeyWasPressed = false;
    }

    static bool bF1WasPressed = false;
    if (glfwGetKey(m_pWindow, GLFW_KEY_F1) == GLFW_PRESS) {
        if (!bF1WasPressed) {
            bF1WasPressed = true;
            m_bShowHelpWindow = !m_bShowHelpWindow;
            if (m_bShowHelpWindow) {
                glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
#ifdef _WIN32
                glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#else
                glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
#endif
            }
        }
    } else {
        bF1WasPressed = false;
    }
}
//*********************************************************************
