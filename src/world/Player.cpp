#include "Player.h"
#include <iostream>

//*********************************************************************
Player::Player(const Core::Vec3& objStartPos) : m_objCamera(objStartPos) {
    m_objRigidBody.m_ObjPos = objStartPos;
    m_objRigidBody.m_ObjVelocity = Core::Vec3(0.0f, 0.0f, 0.0f);
    m_objRigidBody.m_ObjSize = Core::Vec3(0.3f, 0.9f, 0.3f);
}
//*********************************************************************
void Player::Update(float fDeltaTime, const ChunkManager& objChunkManager) {
    PhysicsSystem::Update(m_objRigidBody, fDeltaTime, objChunkManager);

    Core::Vec3 objEyeOffset(0.0f, 0.6f, 0.0f);
    m_objCamera.SetCameraPosition(m_objRigidBody.m_ObjPos + objEyeOffset);

    if (std::abs(m_objRigidBody.m_ObjVelocity.y) < 0.001f) {
        m_bIsGrounded = true;
    } else
        m_bIsGrounded = false;
}
//*********************************************************************
void Player::ProcessKeyboard(MovementDirection iDir, float fDeltaTime) {
    float fSpeed = m_fMoveSpeed;

    Core::Vec3 objForward = m_objCamera.GetFront();
    objForward.y = 0.0f;
    objForward = objForward.normalize();

    Core::Vec3 objRight = m_objCamera.GetRight();
    objRight.y = 0.0f;
    objRight = objRight.normalize();

    if (iDir == MovementDirection::FORWARD) {
        m_objRigidBody.m_ObjVelocity.x += objForward.x * fSpeed * fDeltaTime * 10.0f;
        m_objRigidBody.m_ObjVelocity.z += objForward.z * fSpeed * fDeltaTime * 10.0f;
    }
    if (iDir == MovementDirection::BACKWARD) {
        m_objRigidBody.m_ObjVelocity.x -= objForward.x * fSpeed * fDeltaTime * 10.0f;
        m_objRigidBody.m_ObjVelocity.z -= objForward.z * fSpeed * fDeltaTime * 10.0f;
    }
    if (iDir == MovementDirection::LEFTSIDE) {
        m_objRigidBody.m_ObjVelocity.x -= objRight.x * fSpeed * fDeltaTime * 10.0f;
        m_objRigidBody.m_ObjVelocity.z -= objRight.z * fSpeed * fDeltaTime * 10.0f;
    }
    if (iDir == MovementDirection::RIGHTSIDE) {
        m_objRigidBody.m_ObjVelocity.x += objRight.x * fSpeed * fDeltaTime * 10.0f;
        m_objRigidBody.m_ObjVelocity.z += objRight.z * fSpeed * fDeltaTime * 10.0f;
    }

    if (iDir == MovementDirection::UPSIDE && m_bIsGrounded) {
        m_objRigidBody.m_ObjVelocity.y = m_fJumpSpeed;
        m_bIsGrounded = false;
    }
}
//*********************************************************************
void Player::ProcessMouseMovement(float fXOffset, float fYOffset, bool bConstraintPitch) {
    m_objCamera.processMouseMovement(fXOffset, fYOffset, bConstraintPitch);
}
//*********************************************************************