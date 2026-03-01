#include "InputHandler.h"
#include <cmath>
#include <iostream>
#include <string>

#include "../renderer/PrimitiveRenderer.h"
#include "InputManager.h"

namespace App {
//*********************************************************************
InputHandler::InputHandler(const Core::Vec3& objStartPos) : m_objCameraPos(objStartPos) {
    m_pPlayer = std::make_unique<Player>(objStartPos);
}
//*********************************************************************
void InputHandler::UpdatePlayerPhysics(float fDeltaTime, const ChunkManager& objChunkManager) {
    if (m_pPlayer && m_bPerspective) {
        m_pPlayer->Update(fDeltaTime, objChunkManager, m_bFlyMode);
    }
}
//*********************************************************************
void InputHandler::ProcessInput(GLFWwindow* pWindow,
                                ChunkManager& objChunkManager,
                                float fDeltaTime) {
    InputManager& inputs = InputManager::GetInstance();

    if (inputs.IsKeyPressed(GLFW_KEY_ESCAPE)) {
        if (!m_bEscClickedFirstTime) {
            objChunkManager.SaveWorld();
        }
        m_bEscClickedFirstTime = true;
        glfwSetWindowShouldClose(pWindow, true);
    }
    if (inputs.IsKeyJustPressed(GLFW_KEY_P)) {
        m_bPerspective = !m_bPerspective;
        if (m_bPerspective)
            glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else
            glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        UpdateTitleInfo(pWindow);
    }
    if (inputs.IsKeyJustPressed(GLFW_KEY_F)) {
        m_bCullingEnabled = !m_bCullingEnabled;
        UpdateTitleInfo(pWindow);
    }

    if (inputs.IsKeyJustPressed(GLFW_KEY_R)) {
        if (m_pPlayer) {
            m_pPlayer->GetCamera().SetCameraPosition(m_objCameraPos);
            m_pPlayer->GetCamera().SetCameraZoom(45.0f);
            m_pPlayer->GetCamera().SetCameraFront(Core::Vec3(0.0f, 0.0f, -1.0f));
            m_pPlayer->GetCamera().SetCameraYawPitch(-90.0f, -30.0f);
            m_pPlayer->GetCamera().UpdateCameraVectors();
        }
    }

    double scrollY = inputs.GetScroll();
    Core::Vec3 mouseDelta = inputs.GetMouseDelta();
    if (m_bPerspective) {
        if (inputs.IsKeyPressed(GLFW_KEY_W))
            m_pPlayer->ProcessKeyboard(MovementDirection::FORWARD, fDeltaTime);
        if (inputs.IsKeyPressed(GLFW_KEY_S))
            m_pPlayer->ProcessKeyboard(MovementDirection::BACKWARD, fDeltaTime);
        if (inputs.IsKeyPressed(GLFW_KEY_A))
            m_pPlayer->ProcessKeyboard(MovementDirection::LEFTSIDE, fDeltaTime);
        if (inputs.IsKeyPressed(GLFW_KEY_D))
            m_pPlayer->ProcessKeyboard(MovementDirection::RIGHTSIDE, fDeltaTime);
        if (inputs.IsKeyPressed(GLFW_KEY_SPACE))
            m_pPlayer->ProcessKeyboard(MovementDirection::UPSIDE, fDeltaTime);

        if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f) {
            m_pPlayer->ProcessMouseMovement(mouseDelta.x, mouseDelta.y);
        }
        if (scrollY != 0.0) {
            m_pPlayer->GetCamera().ProcessMouseScroll(static_cast<float>(scrollY));
        }
    } else {
        if (inputs.IsKeyPressed(GLFW_KEY_W))
            m_pPlayer->ProcessKeyboard(MovementDirection::UPSIDE, fDeltaTime);
        if (inputs.IsKeyPressed(GLFW_KEY_S))
            m_pPlayer->ProcessKeyboard(MovementDirection::DOWNSIDE, fDeltaTime);
        if (inputs.IsKeyPressed(GLFW_KEY_A))
            m_pPlayer->ProcessKeyboard(MovementDirection::LEFTSIDE, fDeltaTime);
        if (inputs.IsKeyPressed(GLFW_KEY_D))
            m_pPlayer->ProcessKeyboard(MovementDirection::RIGHTSIDE, fDeltaTime);

        float fZoomSpeed = 10.0f * fDeltaTime;
        if (inputs.IsKeyPressed(GLFW_KEY_SPACE)) {
            m_fOrthoSize -= fZoomSpeed;
            if (m_fOrthoSize < 1.0f)
                m_fOrthoSize = 1.0f;
        }
        if (inputs.IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
            m_fOrthoSize += fZoomSpeed;
            if (m_fOrthoSize > 40.0f)
                m_fOrthoSize = 40.0f;
        }

        if (inputs.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f) {
                m_pPlayer->ProcessMouseMovement(mouseDelta.x, mouseDelta.y);
            }
        } else if (inputs.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
            if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f) {
                m_pPlayer->GetCamera().ProcessMousePan(mouseDelta.x, mouseDelta.y);
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
}
//*********************************************************************
RayHit InputHandler::ProcessFirePreviewAndFire(ChunkManager& objChunkManager,
                                               const Core::Mat4& viewProjection) {
    RayHit objRayHit;
    InputManager& inputs = InputManager::GetInstance();

    // Ctrl Key enables "Debug/Building" mode
    if (inputs.IsKeyPressed(GLFW_KEY_LEFT_CONTROL) || inputs.IsKeyPressed(GLFW_KEY_RIGHT_CONTROL)) {
        glDisable(GL_DEPTH_TEST);
        float fMaxDistance = 60.0f;
        Core::Ray objRay(GetCamera().GetCameraPosition(), GetCamera().GetFront());

        // Draw Debug Ray
        Core::Vec3 objUp = GetCamera().GetUp();
        Core::Vec3 objRight = GetCamera().GetFront().cross(objUp).normalize();
        Core::Vec3 objRayStart = objRay.m_objPtOrigin - objUp * 0.1f + objRight * 0.2f;
        Core::Vec3 objRayEnd = objRay.at(fMaxDistance);
        Renderer::PrimitiveRenderer::DrawLine(
            objRayStart, objRayEnd, Core::Vec3(1.0f, 1.0f, 0.0f), viewProjection);

        objRayHit = PhysicsSystem::RayCast(objRay, fMaxDistance, objChunkManager);
        if (objRayHit.m_bHit) {
            Core::Vec3 objBlockPos(static_cast<float>(objRayHit.m_iBlocKX),
                                   static_cast<float>(objRayHit.m_iBlocKY),
                                   static_cast<float>(objRayHit.m_iBlocKZ));

            // Draw Block Highlight
            Renderer::PrimitiveRenderer::DrawCube(objBlockPos,
                                                  Core::Vec3(1.005f, 1.005f, 1.005f),
                                                  Core::Vec3(1.0f, 0.0f, 1.0f),
                                                  viewProjection);

            // Destroy Block
            if (inputs.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
                if (m_bLMBClickedFirstTime) {
                    objChunkManager.SetBlock(
                        objRayHit.m_iBlocKX, objRayHit.m_iBlocKY, objRayHit.m_iBlocKZ, 0);
                    m_bLMBClickedFirstTime = false;
                }
            } else {
                m_bLMBClickedFirstTime = true;
            }

            // Place Block
            if (inputs.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
                if (m_bRMBClickedFirstTime) {
                    int iBlockX = objRayHit.m_iBlocKX + static_cast<int>(objRayHit.m_objNormal.x);
                    int iBlockY = objRayHit.m_iBlocKY + static_cast<int>(objRayHit.m_objNormal.y);
                    int iBlockZ = objRayHit.m_iBlocKZ + static_cast<int>(objRayHit.m_objNormal.z);
                    objChunkManager.SetBlock(iBlockX, iBlockY, iBlockZ, 1);
                    m_bRMBClickedFirstTime = false;
                }
            } else {
                m_bRMBClickedFirstTime = true;
            }
            if (inputs.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
                if (m_bMMBClickedFirstTime) {
                    int iWorldX = objRayHit.m_iBlocKX + static_cast<int>(objRayHit.m_objNormal.x);
                    int iWorldY = objRayHit.m_iBlocKY + static_cast<int>(objRayHit.m_objNormal.y);
                    int iWorldZ = objRayHit.m_iBlocKZ + static_cast<int>(objRayHit.m_objNormal.z);
                    int iChunkX =
                        static_cast<int>(std::floor(static_cast<float>(iWorldX) / CHUNK_SIZE));
                    int iChunkZ =
                        static_cast<int>(std::floor(static_cast<float>(iWorldZ) / CHUNK_SIZE));

                    Chunk* pChunk = objChunkManager.GetChunk(iChunkX, iChunkZ);
                    // Convert to local coords
                    int iLocalX = iWorldX % CHUNK_SIZE;
                    int iLocalZ = iWorldZ % CHUNK_SIZE;
                    if (iLocalX < 0)
                        iLocalX += CHUNK_SIZE;
                    if (iLocalZ < 0)
                        iLocalZ += CHUNK_SIZE;
                    if (iWorldY == CHUNK_HEIGHT)
                        iWorldY--;

                    if (pChunk) {
                        pChunk->InjectHeat(iLocalX, iWorldY, iLocalZ, 5000.0f);
                    }
                    m_bMMBClickedFirstTime = false;
                }
            } else {
                m_bMMBClickedFirstTime = true;
            }
        }
        glEnable(GL_DEPTH_TEST);
    }
    return objRayHit;
}
//*********************************************************************
Core::Mat4 InputHandler::GetViewProjectionMatrix() {
    Core::Mat4 projection;
    float fAspectRatio = static_cast<float>(m_iScreenWidth) / static_cast<float>(m_iScreenHeight);
    if (App::InputHandler::IsPerspective()) {
        projection = Core::Mat4::Perspective(GetCamera().GetZoom(), fAspectRatio, 0.1f, 100.0f);
    } else {
        float fHeight = m_fOrthoSize;
        float fWidth = m_fOrthoSize * fAspectRatio;
        projection = Core::Mat4::Orthographic(-fWidth, fWidth, -fHeight, fHeight, -100.0f, 100.0f);
    }
    Core::Mat4 view = GetCamera().GetViewMatrix();
    return projection * view;
}
//*********************************************************************
void InputHandler::UpdateTitleInfo(GLFWwindow* pWindow) const {
    if (!pWindow)
        return;

    int iFPS =
        (m_fTimer > 0.0f) ? static_cast<int>(static_cast<float>(m_iFrameCount) / m_fTimer) : 0;

    std::string strTitle = "HPC Voxel Engine FPS:" + std::to_string(iFPS);
    strTitle += (m_bCullingEnabled ? "\tCulling: ON (F)" : "\tCulling: OFF (F)");
    strTitle += (m_bPerspective ? "\tMode: Perspective (P)" : "\tMode: Orthographic (P)");

    glfwSetWindowTitle(pWindow, strTitle.c_str());
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