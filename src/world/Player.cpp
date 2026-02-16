#include "Player.h"
#include <iostream>

//*********************************************************************
Player::Player(const Core::Vec3& objStartPos) : m_objCamera(objStartPos) {
    m_objRigidBody.m_ObjPos = objStartPos;
    m_objRigidBody.m_ObjVelocity = Core::Vec3(0.0f, 0.0f, 0.0f);
    // Player Hitbox Size (Width: 0.6, Height: 1.8)
    m_objRigidBody.m_ObjSize = Core::Vec3(0.3f, 0.9f, 0.3f);
}
//*********************************************************************
void Player::Update(float fDeltaTime, const ChunkManager& objChunkManager, bool bFlyMode) {
    m_bFlyMode = bFlyMode;

    // Step 1: Run Physics
    if (m_bFlyMode) {
        // --- FIXED ALTITUDE LOGIC ---
        // 1. Force Velocity Y to 0 so physics doesn't try to apply gravity
        m_objRigidBody.m_ObjVelocity.y = 0.0f;

        // 2. Set fixed height (e.g., Chunk Height + Offset)
        // Adjust '22.0f' to whatever fixed height you prefer (e.g., CHUNK_HEIGHT + 6.0f)
        m_objRigidBody.m_ObjPos.y = 22.0f;

        // 3. Update only Horizontal Physics (Collision checks for walls still apply)
        PhysicsSystem::Update(m_objRigidBody, fDeltaTime, objChunkManager, bFlyMode);
    } else {
        // Normal Physics (Gravity + Collision)
        PhysicsSystem::Update(m_objRigidBody, fDeltaTime, objChunkManager, bFlyMode);
    }
    // Step 2: Sync Camera to Physics Body (with eye offset)
    Core::Vec3 objEyeOffset(0.0f, 0.6f, 0.0f);  // Eyes are near top of head
    m_objCamera.SetCameraPosition(m_objRigidBody.m_ObjPos + objEyeOffset);

    m_bIsGrounded = m_objRigidBody.m_bIsGrounded;
}
//*********************************************************************
void Player::ProcessKeyboard(MovementDirection iDir, float fDeltaTime) {
    float fSpeed = m_fMoveSpeed;

    // Get Camera vectors
    Core::Vec3 objForward = m_objCamera.GetFront();
    Core::Vec3 objRight = m_objCamera.GetRight();

    // Determine Input Force multiplier
    // Air Control is weaker than Ground Control
    float fInputForce = (m_objRigidBody.m_bIsGrounded || m_bFlyMode) ? 10.0f : 2.0f;

    objForward.y = 0.0f;
    objForward = objForward.normalize();

    objRight.y = 0.0f;
    objRight = objRight.normalize();
    // For both Walk and Fly Mode Logic
    if (iDir == MovementDirection::FORWARD) {
        m_objRigidBody.m_ObjVelocity.x += objForward.x * fSpeed * fDeltaTime * fInputForce;
        m_objRigidBody.m_ObjVelocity.z += objForward.z * fSpeed * fDeltaTime * fInputForce;
    }
    if (iDir == MovementDirection::BACKWARD) {
        m_objRigidBody.m_ObjVelocity.x -= objForward.x * fSpeed * fDeltaTime * fInputForce;
        m_objRigidBody.m_ObjVelocity.z -= objForward.z * fSpeed * fDeltaTime * fInputForce;
    }
    if (iDir == MovementDirection::LEFTSIDE) {
        m_objRigidBody.m_ObjVelocity.x -= objRight.x * fSpeed * fDeltaTime * fInputForce;
        m_objRigidBody.m_ObjVelocity.z -= objRight.z * fSpeed * fDeltaTime * fInputForce;
    }
    if (iDir == MovementDirection::RIGHTSIDE) {
        m_objRigidBody.m_ObjVelocity.x += objRight.x * fSpeed * fDeltaTime * fInputForce;
        m_objRigidBody.m_ObjVelocity.z += objRight.z * fSpeed * fDeltaTime * fInputForce;
    }

    // Jumping
    if (!m_bFlyMode) {
        if (iDir == MovementDirection::UPSIDE && m_bIsGrounded) {
            m_objRigidBody.m_ObjVelocity.y = m_fJumpSpeed;  // Instant upward velocity
            m_objRigidBody.m_bIsGrounded = false;
            m_bIsGrounded = false;
        }
    }
}
//*********************************************************************
void Player::ProcessMouseMovement(float fXOffset, float fYOffset, bool bConstraintPitch) {
    m_objCamera.ProcessMouseMovement(fXOffset, fYOffset, bConstraintPitch);
}
//*********************************************************************