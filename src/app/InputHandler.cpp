#include "InputHandler.h"
#include "../physics/PhysicsSystem.h"
#include "../renderer/PrimitiveRenderer.h"
#include "InputManager.h"

namespace App {
const unsigned int InputHandler::SCREEN_WIDTH = 1280;
const unsigned int InputHandler::SCREEN_HEIGHT = 720;
float InputHandler::m_fLastX = SCREEN_WIDTH / 2.0f;
float InputHandler::m_fLastY = SCREEN_HEIGHT / 2.0f;
float InputHandler::m_fOrthoSize = 10.0f;
float InputHandler::m_fTimer = 0.0f;
float InputHandler::m_fDeltaTime = 0.0f;
int InputHandler::m_iFrameCount = 0;
bool InputHandler::m_bFirstMouse = true;
bool InputHandler::m_bEnableFaceCulling = false;
bool InputHandler::m_bPerspective = true;
bool InputHandler::m_bLMBClickedFirstTime = false;
bool InputHandler::m_bFKeyPressedLastTime = false;
bool InputHandler::m_bPKeyPressedLastTime = false;
Core::Vec3 InputHandler::m_objCameraPos = Core::Vec3(100.0f, 20.0f, 140.0f);
Core::Camera InputHandler::m_objCamera(m_objCameraPos);

//*********************************************************************
void InputHandler::ProcessInput(GLFWwindow* pWindow, float fDeltaTime) {
    InputManager& inputs = InputManager::GetInstance();
    if (inputs.IsKeyPressed(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(pWindow, true);
    if (inputs.IsKeyJustPressed(GLFW_KEY_P)) {
        m_bPerspective = !m_bPerspective;
        if (m_bPerspective)
            glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else
            glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        UpdateTitleInfo(pWindow);
    }
    if (inputs.IsKeyJustPressed(GLFW_KEY_F)) {
        m_bEnableFaceCulling = !m_bEnableFaceCulling;
        UpdateTitleInfo(pWindow);
    }

    if (inputs.IsKeyJustPressed(GLFW_KEY_R)) {
        m_objCamera.SetCameraPosition(m_objCameraPos);
        m_objCamera.SetCameraZoom(45.0f);
        m_objCamera.SetCameraFront(Core::Vec3(0.0f, 0.0f, -1.0f));
        m_objCamera.SetCameraYawPitch(-90.0f, -30.0f);
        m_objCamera.UpdateCameraVectors();
    }

    double scrollY = inputs.GetScroll();
    Core::Vec3 mouseDelta = inputs.GetMouseDelta();
    if (m_bPerspective) {
        if (inputs.IsKeyPressed(GLFW_KEY_W))
            m_objCamera.processKeyboard(0, m_fDeltaTime);
        if (inputs.IsKeyPressed(GLFW_KEY_S) == GLFW_PRESS)
            m_objCamera.processKeyboard(1, m_fDeltaTime);
        if (inputs.IsKeyPressed(GLFW_KEY_SPACE))
            m_objCamera.processKeyboard(4, m_fDeltaTime);
        if (inputs.IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
            m_objCamera.processKeyboard(5, m_fDeltaTime);
        // Always rotate (FPS Style) - Use this if cursor is hidden
        if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f) {
            m_objCamera.processMouseMovement(mouseDelta.x, mouseDelta.y);
        }
        if (scrollY != 0.0) {
            m_objCamera.ProcessMouseScroll(static_cast<float>(scrollY));
        }
    } else {
        if (inputs.IsKeyPressed(GLFW_KEY_W))
            m_objCamera.processKeyboard(4, m_fDeltaTime);
        if (inputs.IsKeyPressed(GLFW_KEY_S))
            m_objCamera.processKeyboard(5, m_fDeltaTime);
        float fZoomSpeed = 10.0f * fDeltaTime;
        if (inputs.IsKeyPressed(GLFW_KEY_SPACE)) {
            m_fOrthoSize -= fZoomSpeed;  // Zoom In
            if (m_fOrthoSize < 1.0f)
                m_fOrthoSize = 1.0f;
        }
        if (inputs.IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
            m_fOrthoSize += fZoomSpeed;  // Zoom Out
            if (m_fOrthoSize > 40.0f)
                m_fOrthoSize = 40.0f;
        }
        // Only rotate when Left Click is held (Editor Style)
        if (inputs.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f) {
                m_objCamera.processMouseMovement(mouseDelta.x, mouseDelta.y);
            }
        } else if (inputs.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
            if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f) {
                m_objCamera.processMousePan(mouseDelta.x, mouseDelta.y);
            }
        }
        if (scrollY != 0.0) {
            m_fOrthoSize -= static_cast<float>(scrollY);
            if (m_fOrthoSize < 1.0f)
                m_fOrthoSize = 1.0f;
            if (m_fOrthoSize > 40.0f)
                m_fOrthoSize = 40.0f;
        }
    }

    if (inputs.IsKeyPressed(GLFW_KEY_A))
        m_objCamera.processKeyboard(2, m_fDeltaTime);
    if (inputs.IsKeyPressed(GLFW_KEY_D))
        m_objCamera.processKeyboard(3, m_fDeltaTime);
}
//*********************************************************************
void InputHandler::processFirePreviewAndFire(std::vector<Chunk>& chunks,
                                             const Core::Mat4& viewProjection) {
    InputManager& inputs = InputManager::GetInstance();
    glDisable(GL_DEPTH_TEST);
    if (inputs.IsKeyPressed(GLFW_KEY_LEFT_CONTROL) || inputs.IsKeyPressed(GLFW_KEY_RIGHT_CONTROL)) {
        float fMaxDistance = 60.0f;
        Core::Ray objRay(App::InputHandler::GetCamera().GetCameraPosition(),
                         App::InputHandler::GetCamera().GetFront());

        // Draw Ray
        Core::Vec3 objUp = App::InputHandler::GetCamera().GetUp();
        Core::Vec3 objRight = App::InputHandler::GetCamera().GetFront().cross(objUp).normalize();
        Core::Vec3 objRayStart = objRay.m_objPtOrigin - objUp * 0.1f + objRight * 0.2f;
        Core::Vec3 objRayEnd = objRay.at(fMaxDistance);
        Renderer::PrimitiveRenderer::DrawLine(
            objRayStart, objRayEnd, Core::Vec3(1.0f, 1.0f, 0.0f), viewProjection);

        RayHit objRayHit = PhysicsSystem::RayCast(objRay, fMaxDistance, chunks);
        if (objRayHit.m_bHit) {
            Core::Vec3 objBlockPos(static_cast<float>(objRayHit.m_iBlocKX),
                                   static_cast<float>(objRayHit.m_iBlocKY),
                                   static_cast<float>(objRayHit.m_iBlocKZ));
// clang-format off
#if DEBUG
            std::cout << "Ray : "
                      << "X: " << objRay.m_objPtOrigin.x << ", "
                      << "Y: " << objRay.m_objPtOrigin.y << ", "
                      << "Z: " << objRay.m_objPtOrigin.z << ", " << objRay.m_objDirection.x << ", "
                      << objRay.m_objDirection.y << ", " << objRay.m_objDirection.z << std::endl;

            std::cout << "Ray Hit! Block: "
                      << "X: " << objRayHit.m_objHitPoint.x << ", "
                      << "Y: " << objRayHit.m_objHitPoint.y << ", "
                      << "Z: " << objRayHit.m_objHitPoint.z << ", " << objRayHit.m_fDistance << ", "
                      << objRayHit.m_iBlocKX << ", " << objRayHit.m_iBlocKY << ", "
                      << objRayHit.m_iBlocKZ << std::endl;
#endif
// clang-format on
            Renderer::PrimitiveRenderer::DrawCube(objBlockPos,
                                                  Core::Vec3(1.005f, 1.005f, 1.005f),
                                                  Core::Vec3(1.0f, 0.0f, 1.0f),
                                                  viewProjection);
        }
        if (inputs.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            if (m_bLMBClickedFirstTime && objRayHit.m_bHit) {
                int iTargetBlockX =
                    static_cast<int>(floor(static_cast<float>(objRayHit.m_iBlocKX) / CHUNK_SIZE));
                int iTargetBlockZ =
                    static_cast<int>(floor(static_cast<float>(objRayHit.m_iBlocKZ) / CHUNK_SIZE));
                for (auto& chunk : chunks) {
                    if (chunk.GetChunkX() == iTargetBlockX && chunk.GetChunkZ() == iTargetBlockZ) {
                        int iLocalX = objRayHit.m_iBlocKX - (iTargetBlockX * CHUNK_SIZE);
                        int iLocalZ = objRayHit.m_iBlocKZ - (iTargetBlockZ * CHUNK_SIZE);
                        chunk.SetBlockAt(iLocalX, objRayHit.m_iBlocKY, iLocalZ, 0);
                        chunk.GenerateMesh();
                        // chunk.Render();
                    }
                }
                m_bLMBClickedFirstTime = false;
            }
        } else
            m_bLMBClickedFirstTime = true;
    }
    glEnable(GL_DEPTH_TEST);
}
//*********************************************************************
void InputHandler::UpdateTitleInfo(GLFWwindow* pWindow) {
    if (!pWindow)
        return;
    int iFPS = static_cast<int>(m_iFrameCount / m_fTimer);
    std::string strTitle = "HPC Voxel Engine FPS:" + std::to_string(iFPS);
    if (m_bEnableFaceCulling)
        strTitle += "\tFace Culling Enabled (Press 'F' key to toogle)";
    else
        strTitle += "\tFace Culling Disabled (Press 'F' key to toogle)";

    if (m_bPerspective)
        strTitle += "\tPerspective Projection (Press 'P' key to toggle)";
    else
        strTitle += "\tOrthographic Projection (Press 'P' key to toggle)";
    glfwSetWindowTitle(pWindow, strTitle.c_str());
}
//*********************************************************************
Core::Mat4 InputHandler::GetViewProjectionMatrix() {
    Core::Mat4 projection;
    float fAspectRatio = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);
    if (App::InputHandler::IsPerspective()) {
        projection = Core::Mat4::perspective(m_objCamera.GetZoom(), fAspectRatio, 0.1f, 100.0f);
    } else {
        float fHeight = m_fOrthoSize;
        float fWidth = m_fOrthoSize * fAspectRatio;
        projection = Core::Mat4::orthographic(-fWidth, fWidth, -fHeight, fHeight, -100.0f, 100.0f);
    }
    Core::Mat4 view = m_objCamera.GetViewMatrix();
    return projection * view;
}
//*********************************************************************
void InputHandler::SetDeltaTime(float fDeltaTime) {
    m_fDeltaTime = fDeltaTime;
    m_fTimer += m_fDeltaTime;
}
//*********************************************************************
void InputHandler::ResetCounters() {
    m_iFrameCount = 0;
    m_fTimer = 0.0f;
}
//*********************************************************************
}  // namespace App