#include <iostream>
#include <vector>
constexpr int BENCHMARK = 0;

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
#include "app/InputHandler.h"
#include "app/InputManager.h"
#include "renderer/WorldRenderer.h"

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
    glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderor = glGetString(GL_RENDERER);
    std::cout << "GPU Renderer: " << vendor << std::endl;
    std::cout << "Renderer: " << renderor << std::endl;

    Renderer::Shader shader("../assets/shaders/vertex.glsl", "../assets/shaders/fragment.glsl");
    Renderer::PrimitiveRenderer::Init();

    std::vector<Chunk> chunks;
    int iRenderDistance = 8;
    int iTotalChunks = (iRenderDistance * 2) * (iRenderDistance * 2);
    chunks.reserve(static_cast<size_t>(iTotalChunks) + 10);
    for (int iX = -iRenderDistance; iX < iRenderDistance; iX++) {
        for (int iZ = -iRenderDistance; iZ < iRenderDistance; iZ++) {
            chunks.emplace_back(iX, iZ);
        }
    }

    // Renderer::Texture texture("../assets/textures/container.jpg");
    Renderer::Texture texture("../assets/textures/texture_atlas.png");
    texture.Bind(0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    float fLastFrame = static_cast<float>(glfwGetTime());
    while (!glfwWindowShouldClose(pWindow)) {
        glClearColor(0.2f, 0.3f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // For FPS Counter
        float fCurrentFrame = static_cast<float>(glfwGetTime());
        float fDeltaTime = fCurrentFrame - fLastFrame;
        fLastFrame = fCurrentFrame;
        inputHandler.AddFrameCount();
        inputHandler.SetDeltaTime(fDeltaTime);
        if (inputHandler.GetTime() >= 1.0f) {
            inputHandler.UpdateTitleInfo(pWindow);
            inputHandler.ResetCounters();
        }

        InputManager::GetInstance().Update();
        glfwPollEvents();
        inputHandler.ProcessInput(pWindow, fDeltaTime);

        inputHandler.UpdatePlayerPhysics(fDeltaTime, chunks);

        Core::Mat4 viewProjection = inputHandler.GetViewProjectionMatrix();
        Renderer::WorldRenderer::DrawChunks(chunks, shader, viewProjection);
        Renderer::WorldRenderer::DrawAxes(viewProjection);
        inputHandler.processFirePreviewAndFire(chunks, viewProjection);

        glfwSwapBuffers(pWindow);
    }
    Renderer::PrimitiveRenderer::Shutdown();
    glfwTerminate();
    return 0;
}
