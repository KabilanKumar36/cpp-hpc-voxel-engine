#pragma once

#include "../core/camera.h"
#include "../physics/PhysicsSystem.h"

// Forward Declaration
class ChunkManager;

enum MovementDirection { FORWARD = 0, BACKWARD, LEFTSIDE, RIGHTSIDE, UPSIDE, DOWNSIDE };

/**
 * @class Player
 * @brief Represents the user-controlled entity.
 * Handles Camera synchronization and Input-to-Physics logic.
 */
class Player {
public:
    Player(const Core::Vec3& objStartPos);

    void Update(float fDeltaTime, const ChunkManager& objChunkManager, bool bFlyMode = false);

    void ProcessKeyboard(MovementDirection iDir, float fDeltaTime);
    void ProcessMouseMovement(float fXOffset, float fYOffset, bool bConstraintPitch = true);

    // Getters & Setters
    Core::Camera& GetCamera() { return m_objCamera; }
    Core::Vec3 GetPosition() const { return m_objRigidBody.m_ObjPos; }
    void SetMovementSpeed(float fMoveSpeed) { m_fMoveSpeed = fMoveSpeed; }

private:
    Core::Camera m_objCamera;
    RigidBody m_objRigidBody;

    float m_fMoveSpeed = 5.0f;
    float m_fJumpSpeed = 8.0f;
    bool m_bIsGrounded = false;

    // Tracks if we are flying for physics calculations
    bool m_bFlyMode = false;
};