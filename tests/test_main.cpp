// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <gtest/gtest.h>
#include <iostream>

#if defined(__linux__) || defined(__APPLE__)
#include <sanitizer/lsan_interface.h>

// Suppress known leaks in Linux GL drivers (Mesa/Nvidia) to keep CI clean
extern "C" const char* __lsan_default_suppressions() {
    return "leak:_glfwInitGLX\n"
           "leak:extensionSupportedGLX\n"
           "leak:glX\n"
           "leak:swrast\n";
}
#endif

// Error callback for GLFW
static void error_callback(int iError, const char* pcMsg) {
    std::cerr << "GLFW Error [" << iError << "]: " << pcMsg << std::endl;
}

// Global Test Environment
class OpenGLEnv : public ::testing::Environment {
public:
    GLFWwindow* pWindow = nullptr;

    void SetUp() override {
        glfwSetErrorCallback(error_callback);

        if (!glfwInit()) {
            std::cerr << "FATAL: Failed to init GLFW for unit tests." << std::endl;
            exit(EXIT_FAILURE);
        }

        // Create a hidden window for context
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        // Try OpenGL 4.6 first
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        pWindow = glfwCreateWindow(640, 480, "Hidden Test Window", NULL, NULL);

        // Fallback to 4.5 if 4.6 is unavailable
        if (!pWindow) {
            std::cerr << "Warning: OpenGL 4.6 not supported. Retrying with 4.5..." << std::endl;
            glfwDefaultWindowHints();
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            pWindow = glfwCreateWindow(640, 480, "Hidden Test Window", NULL, NULL);
            if (!pWindow) {
                std::cerr << "FATAL: Failed to create GLFW Window." << std::endl;
                glfwTerminate();
                exit(EXIT_FAILURE);
            }
        }

        glfwMakeContextCurrent(pWindow);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "FATAL: Failed to load GLAD." << std::endl;
            glfwDestroyWindow(pWindow);
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
    }

    void TearDown() override {
        if (pWindow) {
            glfwDestroyWindow(pWindow);
            pWindow = nullptr;
        }
        glfwTerminate();
    }
};

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    // Register the OpenGL environment
    ::testing::AddGlobalTestEnvironment(new OpenGLEnv);

    return RUN_ALL_TESTS();
}