#pragma once

#include <vector>
#include "../core/MathUtils.h"
#include "../core/Ray.h"
#include "AABB.h"

// Forward declaration: We don't need the full file here, just the name.
// This speeds up compilation significantly.
class ChunkManager;

/**
 * @struct RigidBody
 * @brief Represents a physical object (Player) in the world.
 */
struct RigidBody {
    Core::Vec3 m_ObjPos;
    Core::Vec3 m_ObjVelocity;
    Core::Vec3 m_ObjSize;  // Half extents (Width/2, Height/2, Length/2)
    bool m_bIsGrounded{false};

    AABB GetAABB(const Core::Vec3& newPos) const {
        return AABB(newPos - m_ObjSize, newPos + m_ObjSize);
    }
};

/**
 * @struct RayHit
 * @brief Result of a raycast query.
 */
struct RayHit {
    Core::Vec3 m_objHitPoint{};
    Core::Vec3 m_objNormal{};
    float m_fDistance{0.0f};
    int m_iBlocKX{0}, m_iBlocKY{0}, m_iBlocKZ{0};
    bool m_bHit{false};
};

/**
 * @class PhysicsSystem
 * @brief Static system for managing collisions and raycasting.
 */
class PhysicsSystem {
public:
    /**
     * @brief Updates physics (Gravity, Friction, Velocity).
     * @param objRigidBody The body to update.
     * @param fDeltaTime Time since last frame.
     * @param objChunkManager World data for collision checks.
     * @param bFlyMode If true, disables gravity.
     */
    static void Update(RigidBody& objRigidBody,
                       float fDeltaTime,
                       const ChunkManager& objChunkManager,
                       bool bFlyMode = false);

    /**
     * @brief Performs a DDA Raycast to find the first block hit.
     */
    static RayHit RayCast(const Core::Ray& objRay,
                          float fMaxDistance,
                          const ChunkManager& objChunkManager);

private:
    // Helper: Box-to-Voxel collision
    static bool checkCollision(const AABB& objAABB, const ChunkManager& objChunkManager);

    // Helper: Linear Interpolation
    static float lerp(float a, float b, float t) { return a + (b - a) * t; }
};