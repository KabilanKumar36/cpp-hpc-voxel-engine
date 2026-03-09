#if defined(_WIN32) && defined(NDEBUG)
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

#include <iostream>
#include <vector>

constexpr float CLEAR_COLOR[4] = {0.2f, 0.3f, 0.2f, 1.0f};  // Forest Green color
constexpr float FIXED_THERMAL_TIME_STEP = 1.0f / 60.0f;     // 60 FPS fixed timestep
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
#include "physics/ThermalSystem.h"
#include "renderer/WorldRenderer.h"

static void SetOpenGLState() {
    glEnable(GL_DEPTH_TEST);
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
    App::InputHandler inputHandler(Core::Vec3(100.0f, 40.0f, 100.0f));
#ifdef NDEBUG
    GLFWmonitor* pPrimaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* pVideoMode = glfwGetVideoMode(pPrimaryMonitor);
    glfwWindowHint(GLFW_RED_BITS, pVideoMode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, pVideoMode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, pVideoMode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, pVideoMode->refreshRate);

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    inputHandler.SetScreenWidth(pVideoMode->width, pVideoMode->height);
#endif
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow* pWindow =
        glfwCreateWindow(inputHandler.GetScreenWidth(),
                         inputHandler.GetScreenHeight(),
                         "HPC Voxel Engine ( Esc - Close & F1 - Help & Mouse Control)",
                         /*pPrimaryMonitor*/ nullptr,
                         nullptr);
    if (pWindow == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(pWindow);
    glfwSetWindowTitle(pWindow, "HPC Voxel Engine");

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
        shader.SetInt("u_ThermalTexture", 1);
        Renderer::Texture texture("assets/textures/texture_atlas.png");
        texture.Bind(0);

        // World & Chunk Initialization
        std::string strRegnFilePath = "ChunkData";
        ChunkManager objChunkManager(strRegnFilePath);
        float fLastFrame = static_cast<float>(glfwGetTime());
        float fAccumulator = 0.0f;
        int iMaxThreads = std::thread::hardware_concurrency();
        if (iMaxThreads == 0)
            iMaxThreads = 4;
        int iMainThread = 1;
        int iThermalThreads = (iMaxThreads >= 16) ? 4 : (iMaxThreads >= 8) ? 2 : 1;
        int iRenderingThreads = std::max(1, iMaxThreads - iMainThread - iThermalThreads);
        App.m_iMainThreads = iMainThread;
        App.m_iThermalThreads = iThermalThreads;
        App.m_iMaxRenderingThreads = iRenderingThreads;
        App.m_iActiveThreads = iRenderingThreads;
        ThermalSystem objThermalSystem{iThermalThreads};
        inputHandler.SetActiveThreads(iRenderingThreads);
        objChunkManager.SetActiveThreads(iRenderingThreads);

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
            fAccumulator += fDeltaTime;

            Core::Vec3 objCameraPos = inputHandler.GetCamera().GetCameraPosition();
            objChunkManager.Update(objCameraPos.x, objCameraPos.z);

            // Update Stats
            inputHandler.AddFrameCount();
            inputHandler.SetDeltaTime(fDeltaTime);
            if (inputHandler.GetTime() >= 1.0f) {
                inputHandler.ResetCounters();
            }

            InputManager::GetInstance().Update();
            glfwPollEvents();
            App.HandleUIToggle();
            if (!App.m_bShowHelpWindow) {
                inputHandler.ProcessInput(pWindow, objChunkManager, fDeltaTime);
                App.m_bFrustumCulling = inputHandler.IsFrustumCullingEnabled();
            }
            objThermalSystem.SetEnableSIMD(inputHandler.IsSIMDEnabled());
            int iPhysicsSteps = 0;
            // Fixed timestep loop for thermal simulation to ensure stability
            while (fAccumulator >= FIXED_THERMAL_TIME_STEP) {
                // Logic & Physics
                if (objChunkManager.GetMutableChunks().empty())
                    inputHandler.GetCamera().SetCameraPosition(Core::Vec3(100.0f, 40.0f, 100.0f));
                else
                    inputHandler.UpdatePlayerPhysics(FIXED_THERMAL_TIME_STEP, objChunkManager);
                objThermalSystem.UpdateTemperature(FIXED_THERMAL_TIME_STEP, objChunkManager);

                fAccumulator -= FIXED_THERMAL_TIME_STEP;
                iPhysicsSteps++;
            }
            App.m_iPhysicsSteps = iPhysicsSteps;
            App.m_fAccumulator = fAccumulator;
            // Rendering
            Core::Mat4 viewProjection = inputHandler.GetViewProjectionMatrix();
            Renderer::WorldRenderer::DrawChunks(
                objChunkManager, shader, viewProjection, inputHandler.IsFrustumCullingEnabled());
            Renderer::WorldRenderer::DrawAxes(viewProjection);

            RayHit objRayHit =
                inputHandler.ProcessFirePreviewAndFire(objChunkManager, viewProjection);
            // UI Rendering
            App.BeginImGUIFrame();
            App.RenderMetricsUI(inputHandler, objChunkManager, objRayHit);
            App.RenderHelpUI();
            App.EndImGUIFrame();

            if (inputHandler.IsNeighborCullingEnabled() != objChunkManager.GetNeighborCulling()) {
                objChunkManager.SetNeighborCulling(inputHandler.IsNeighborCullingEnabled());
                objChunkManager.ReloadAllChunks();
            }
            if (inputHandler.GetActiveThreads() != objChunkManager.GetActiveThreads()) {
                objChunkManager.SetActiveThreads(inputHandler.GetActiveThreads());
            }

            glfwSwapBuffers(pWindow);
        }
    }
    App.ShutDownImGUI();
    Renderer::PrimitiveRenderer::Shutdown();
    glfwTerminate();

    return 0;
}