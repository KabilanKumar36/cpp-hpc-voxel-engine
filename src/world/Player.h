#pragma once

#include <vector>
#include "../core/camera.h"
#include "../physics/PhysicsSystem.h"
#include "Chunk.h"

enum MovementDirection { FORWARD = 0, BACKWARD, LEFTSIDE, RIGHTSIDE, UPSIDE, DOWNSIDE };
class Player {
public:
    Player(const Core::Vec3& objStartPos);

    void Update(float fDeltaTime, const std::vector<Chunk>& chunks);

    void ProcessKeyboard(int iDir, float fDeltaTime);
    void ProcessMouseMovement(float fOffset, float fYOffset, bool bConstraintPitch = true);

    Core::Camera& GetCamera() { return m_objCamera; }
    Core::Vec3 GetPosition() const { return m_objRigidBody.m_ObjPos; }

private:
    Core::Camera m_objCamera;
    RigidBody m_objRigidBody;

    float m_fMoveSpeed = 5.0f;
    float m_fJumpSpeed = 8.0f;
    bool m_bIsGrounded = false;
};