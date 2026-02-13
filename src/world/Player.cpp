#include "Player.h"
#include <iostream>

//*********************************************************************
Player::Player(const Core::Vec3& objStartPos) : m_objCamera(objStartPos) {
    m_objRigidBody.m_ObjPos = objStartPos;
    m_objRigidBody.m_ObjVelocity = Core::Vec3(0.0f, 0.0f, 0.0f);
    m_objRigidBody.m_ObjSize = Core::Vec3(0.3f, 0.9f, 0.3f);
}
//*********************************************************************
void Player::Update(float fDeltaTime, const ChunkManager& objChunkManager, bool bFlyMode) {
    PhysicsSystem::Update(m_objRigidBody, fDeltaTime, objChunkManager, bFlyMode);

    Core::Vec3 objEyeOffset(0.0f, 0.6f, 0.0f);
    m_objCamera.SetCameraPosition(m_objRigidBody.m_ObjPos + objEyeOffset);

    m_bIsGrounded = m_objRigidBody.m_bIsGrounded;
}
//*********************************************************************
void Player::ProcessKeyboard(MovementDirection iDir, float fDeltaTime, bool bFlyMode) {
    float fSpeed = m_fMoveSpeed;
    Core::Vec3 objForward = m_objCamera.GetFront();
    Core::Vec3 objRight = m_objCamera.GetRight();

    float fInputForce = 2.0f;
    if (m_objRigidBody.m_bIsGrounded)
        fInputForce = 10.0f;
    if (bFlyMode) {
        Core::Vec3 objUp = m_objCamera.GetUp();
        if (iDir == MovementDirection::FORWARD)
            m_objRigidBody.m_ObjVelocity += objForward * fSpeed * fDeltaTime * fInputForce;
        if (iDir == MovementDirection::BACKWARD)
            m_objRigidBody.m_ObjVelocity -= objForward * fSpeed * fDeltaTime * fInputForce;
        if (iDir == MovementDirection::LEFTSIDE)
            m_objRigidBody.m_ObjVelocity -= objRight * fSpeed * fDeltaTime * fInputForce;
        if (iDir == MovementDirection::RIGHTSIDE)
            m_objRigidBody.m_ObjVelocity += objRight * fSpeed * fDeltaTime * fInputForce;
        if (iDir == MovementDirection::UPSIDE)
            m_objRigidBody.m_ObjVelocity += objUp * fSpeed * fDeltaTime * fInputForce;
        if (iDir == MovementDirection::DOWNSIDE)
            m_objRigidBody.m_ObjVelocity -= objUp * fSpeed * fDeltaTime * fInputForce;
    } else {
        objForward.y = 0.0f;
        objForward = objForward.normalize();

        objRight.y = 0.0f;
        objRight = objRight.normalize();

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

        if (iDir == MovementDirection::UPSIDE && m_bIsGrounded) {
            m_objRigidBody.m_ObjVelocity.y = m_fJumpSpeed;
            m_objRigidBody.m_bIsGrounded = false;
            m_bIsGrounded = false;
        }
    }
}
//*********************************************************************
void Player::ProcessMouseMovement(float fXOffset, float fYOffset, bool bConstraintPitch) {
    m_objCamera.processMouseMovement(fXOffset, fYOffset, bConstraintPitch);
}
//*********************************************************************