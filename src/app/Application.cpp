#include "Application.h"
// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

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
            m_bIsUIActive = !m_bIsUIActive;

            // Switch cursor mode based on UI state
            if (m_bIsUIActive) {
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
}
//*********************************************************************
