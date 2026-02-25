#if defined(_WIN32) && defined(NDEBUG)
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

#include <iostream>
#include <vector>

constexpr float CLEAR_COLOR[4] = {0.2f, 0.3f, 0.2f, 1.0f};  // Forest Green color

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <core/MathUtils.h>
#include <core/Matrix.h>

#include <renderer/PrimitiveRenderer.h>
#include <renderer/Shader.h>
#include <renderer/Texture.h>

#include <world/Chunk.h>
#include <world/ChunkManager.h>
#include "app/Application.h"
#include "app/InputHandler.h"
#include "app/InputManager.h"
#include "renderer/WorldRenderer.h"

static void SetOpenGLState() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}
//*********************************************************************
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    GLFWmonitor* pPrimaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* pVideoMode = glfwGetVideoMode(pPrimaryMonitor);
    glfwWindowHint(GLFW_RED_BITS, pVideoMode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, pVideoMode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, pVideoMode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, pVideoMode->refreshRate);

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    App::InputHandler inputHandler(Core::Vec3(100.0f, 40.0f, 100.0f));
    inputHandler.SetScreenWidth(pVideoMode->width, pVideoMode->height);
    GLFWwindow* pWindow = glfwCreateWindow(inputHandler.GetScreenWidth(),
                                           inputHandler.GetScreenHeight(),
                                           "HPC Voxel Engine",
                                           pPrimaryMonitor,
                                           nullptr);
    if (pWindow == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(pWindow);

    // Unlock FPS (VSync OFF) to demonstrate high performance
    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    InputManager::GetInstance().Init(pWindow);
    Application App(pWindow);
    App.InitImGUI();
#ifdef _WIN32
    // Enable High-DPI support on Windows
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#else
    glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
#endif

    // Explicit scoping for openGL items to maintain RAII before glfwTerminate is called to deleted.
    {
        Renderer::Shader shader("assets/shaders/vertex_Chunk.glsl",
                                "assets/shaders/fragment_Chunk.glsl");
        Renderer::PrimitiveRenderer::Init();

        shader.Use();
        shader.SetInt("u_Texture", 0);
        Renderer::Texture texture("assets/textures/texture_atlas.png");
        texture.Bind(0);

        // World & Chunk Initialization
        std::string strRegnFilePath = "ChunkData";
        ChunkManager objChunkManager(strRegnFilePath);
        float fLastFrame = static_cast<float>(glfwGetTime());
        static bool bFlyMode = false;

        // Main Render Loop
        while (!glfwWindowShouldClose(pWindow)) {
            glClearColor(CLEAR_COLOR[0], CLEAR_COLOR[1], CLEAR_COLOR[2], CLEAR_COLOR[3]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            SetOpenGLState();

            float fCurrentFrame = static_cast<float>(glfwGetTime());
            float fDeltaTime = fCurrentFrame - fLastFrame;
            // Cap delta time to prevent physics explosions on lag spikes
            if (fDeltaTime > 0.1f)
                fDeltaTime = 0.1f;
            fLastFrame = fCurrentFrame;
            Core::Vec3 objCameraPos = inputHandler.GetCamera().GetCameraPosition();
            objChunkManager.Update(objCameraPos.x, objCameraPos.z);

            // Update Stats
            inputHandler.AddFrameCount();
            inputHandler.SetDeltaTime(fDeltaTime);
            if (inputHandler.GetTime() >= 1.0f) {
                inputHandler.UpdateTitleInfo(pWindow);
                inputHandler.ResetCounters();
            }

            InputManager::GetInstance().Update();
            glfwPollEvents();
            App.HandleUIToggle();
            if (!App.m_bIsUIActive) {
                inputHandler.ProcessInput(pWindow, objChunkManager, fDeltaTime);
            }

            // Logic & Physics
            if (objChunkManager.GetMutableChunks().empty())
                inputHandler.GetCamera().SetCameraPosition(Core::Vec3(100.0f, 40.0f, 100.0f));
            else
                inputHandler.UpdatePlayerPhysics(fDeltaTime, objChunkManager);

            PhysicsSystem::UpdateTemparature(
                fDeltaTime, inputHandler.GetFrameCount(), objChunkManager);

            // Rendering
            Core::Mat4 viewProjection = inputHandler.GetViewProjectionMatrix();
            Renderer::WorldRenderer::DrawChunks(
                objChunkManager, shader, viewProjection, inputHandler.IsCullingEnabled());
            Renderer::WorldRenderer::DrawAxes(viewProjection);
            RayHit objRayHit =
                inputHandler.ProcessFirePreviewAndFire(objChunkManager, viewProjection);

            // UI Rendering
            App.BeginImGUIFrame();
            if (App.m_bShowDebugPanel) {
                ImGui::SetNextWindowSize(ImVec2(420, 540), ImGuiCond_FirstUseEver);
                ImGui::Begin("System Monitor", &App.m_bShowDebugPanel);

                ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Dev. Tools");
                ImGui::Separator();
                static bool bWireframeMode = false;
                if (ImGui::Checkbox("Wireframe", &bWireframeMode)) {
                    glPolygonMode(GL_FRONT_AND_BACK, bWireframeMode ? GL_LINE : GL_FILL);
                }
                if (ImGui::Checkbox("Fly Mode", &bFlyMode)) {
                    inputHandler.SetFlyMode(bFlyMode);
                }

                static float fFlySpeed = 20.0f;
                if (ImGui::SliderFloat("Fly Speed", &fFlySpeed, 1.0f, 200.0f)) {
                    inputHandler.SetMovementSpeed(fFlySpeed);
                }
                ImGui::Separator();

                ImGui::TextColored(ImVec4(0, 1, 0, 1), "Performance");
                ImGui::Text("FPS: %0.1f", ImGui::GetIO().Framerate);
                ImGui::Text("Frame Time: %0.3f ms", 1000.0f / ImGui::GetIO().Framerate);
                ImGui::Separator();

                Core::Vec3 objCurrCameraPos = inputHandler.GetCamera().GetCameraPosition();
                ImGui::TextColored(ImVec4(0, 1, 1, 1), "Position");
                ImGui::Text("X: %0.3f, Y: %0.3f, Z: %0.3f",
                            objCurrCameraPos.x,
                            objCurrCameraPos.y,
                            objCurrCameraPos.z);

                ImGui::Separator();

                ImGui::TextColored(ImVec4(1, 1, 0, 1), "World State");
                ImGui::Text("Chunks Loaded: %zu", objChunkManager.GetMutableChunks().size());
                ImGui::Separator();

                ImGui::TextColored(ImVec4(1, 0, 1, 1), "Interaction");
                if (objRayHit.m_bHit) {
                    ImGui::Text("Target: Block (%d, %d, %d)",
                                objRayHit.m_iBlocKX,
                                objRayHit.m_iBlocKY,
                                objRayHit.m_iBlocKZ);
                    ImGui::Text("Distance: %.2f", objRayHit.m_fDistance);
                }
                ImGui::End();
            }
            App.EndImGUIFrame();
            glfwSwapBuffers(pWindow);
        }
    }
    App.ShutDownImGUI();
    Renderer::PrimitiveRenderer::Shutdown();
    glfwTerminate();

    return 0;
}