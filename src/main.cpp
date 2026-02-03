#include <iostream>
#include <vector>
#define BENCHMARK 0

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
    GLFWwindow* pWindow = glfwCreateWindow(App::InputHandler::GetScreenWidth(),
                                           App::InputHandler::GetScreenHeight(),
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
        App::InputHandler::AddFrameCount();
        App::InputHandler::SetDeltaTime(fDeltaTime);
        if (App::InputHandler::GetTime() >= 1.0f) {
            App::InputHandler::UpdateTitleInfo(pWindow);
            App::InputHandler::ResetCounters();
        }

        InputManager::GetInstance().Update();
        glfwPollEvents();
        App::InputHandler::ProcessInput(pWindow, fDeltaTime);
        
        Core::Mat4 viewProjection = App::InputHandler::GetViewProjectionMatrix();
        Renderer::WorldRenderer::DrawChunks(chunks, shader, viewProjection);
        Renderer::WorldRenderer::DrawAxes(viewProjection);
        App::InputHandler::processFirePreviewAndFire(chunks, viewProjection);

        glfwSwapBuffers(pWindow);
        //glfwPollEvents();
    }
    Renderer::PrimitiveRenderer::Shutdown();
    glfwTerminate();
    return 0;
}
