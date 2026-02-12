#include <iostream>
#include <vector>
constexpr int BENCHMARK = 0;
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    App::InputHandler inputHandler(Core::Vec3(100.0f, 40.0f, 100.0f));

    GLFWwindow* pWindow = glfwCreateWindow(inputHandler.GetScreenWidth(),
                                           inputHandler.GetScreenHeight(),
                                           "HPC Voxel Engine",
                                           nullptr,
                                           nullptr);
    if (pWindow == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(pWindow);
#if BENCHMARK
    glfwSwapInterval(0);  // 0 = Unlock FPS (VSync OFF), 1 = Lock to 60 (VSync ON)
#endif

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    InputManager::GetInstance().Init(pWindow);
    Application App(pWindow);
    App.InitImGUI();
    glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    std::cout << "GPU Renderer: " << vendor << std::endl;
    std::cout << "Renderer: " << renderer << std::endl;

    Renderer::Shader shader("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl");
    Renderer::PrimitiveRenderer::Init();
    SetOpenGLState();

    shader.Use();
    shader.SetInt("u_Texture", 0);
    Renderer::Texture texture("assets/textures/texture_atlas.png");
    texture.Bind(0);

    ChunkManager objChunkManager;
    float fLastFrame = static_cast<float>(glfwGetTime());
    static bool bFlyMode = false;

    while (!glfwWindowShouldClose(pWindow)) {
        glClearColor(CLEAR_COLOR[0], CLEAR_COLOR[1], CLEAR_COLOR[2], CLEAR_COLOR[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float fCurrentFrame = static_cast<float>(glfwGetTime());
        float fDeltaTime = fCurrentFrame - fLastFrame;
        if (fDeltaTime > 0.1f)
            fDeltaTime = 0.1f;
        fLastFrame = fCurrentFrame;
        Core::Vec3 objCameraPos = inputHandler.GetCamera().GetCameraPosition();
        objChunkManager.Update(objCameraPos.x, objCameraPos.z);

        // For FPS Counter
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
            inputHandler.ProcessInput(pWindow, fDeltaTime);
        }

        if (objChunkManager.GetMutableChunks().empty())
            inputHandler.GetCamera().SetCameraPosition(Core::Vec3(100.0f, 40.0f, 100.0f));
        else
            inputHandler.UpdatePlayerPhysics(fDeltaTime, objChunkManager);

        Core::Mat4 viewProjection = inputHandler.GetViewProjectionMatrix();
        Renderer::WorldRenderer::DrawChunks(objChunkManager, shader, viewProjection);
        Renderer::WorldRenderer::DrawAxes(viewProjection);

        RayHit objRayHit = inputHandler.processFirePreviewAndFire(objChunkManager, viewProjection);
        App.BeginImGUIFrame();
        if (App.m_bShowDebugPanel) {
            ImGui::SetNextWindowSize(ImVec2(280, 360), ImGuiCond_FirstUseEver);
            ImGui::Begin("System Monitor", &App.m_bShowDebugPanel);

            ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Dev. Tools");
            ImGui::Separator();
            static bool bWireframeMode = false;
            if (ImGui::Checkbox("Wireframe", &bWireframeMode)) {
                if (bWireframeMode) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                } else {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }
            }
            if (ImGui::Checkbox("Fly Mode", &bFlyMode)) {
                if (bFlyMode)
                    inputHandler.SetFlyMode(bFlyMode);
                else
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
    App.ShutDownImGUI();
    Renderer::PrimitiveRenderer::Shutdown();
    glfwTerminate();

    return 0;
}
