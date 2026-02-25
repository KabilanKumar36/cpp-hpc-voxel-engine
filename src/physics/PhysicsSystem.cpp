#include "PhysicsSystem.h"
#include <algorithm>
#include <cmath>
#include <limits>

// We only include the heavy headers here, in the implementation.
#include "../world/Chunk.h"
#include "../world/ChunkManager.h"

//*********************************************************************
void PhysicsSystem::Update(RigidBody& objRigidBody,
                           float fDeltaTime,
                           const ChunkManager& objChunkManager,
                           bool bFlyMode) {
    // 1. Physics Constants
    constexpr float fGravity = -20.0f;
    constexpr float fFriction = 10.0f;
    constexpr float fAirDrag = 1.0f;

    // 2. Friction
    // In Fly Mode, we use AirDrag to stop "drifting" when keys are released
    float fCurrentFriction = (objRigidBody.m_bIsGrounded || bFlyMode) ? fFriction : fAirDrag;
    float fAlpha = std::min(fCurrentFriction * fDeltaTime, 1.0f);

    // Apply Friction to Horizontal axes
    objRigidBody.m_ObjVelocity.x = lerp(objRigidBody.m_ObjVelocity.x, 0.0f, fAlpha);
    objRigidBody.m_ObjVelocity.z = lerp(objRigidBody.m_ObjVelocity.z, 0.0f, fAlpha);

    // 3. Gravity & Vertical Movement
    if (!bFlyMode) {
        // Normal Physics: Apply Gravity
        objRigidBody.m_ObjVelocity.y += fGravity * fDeltaTime;
    } else {
        // Fly Mode: No Gravity, apply Friction to Y axis for "Hover" effect
        objRigidBody.m_ObjVelocity.y = lerp(objRigidBody.m_ObjVelocity.y, 0.0f, fAlpha);
    }

    objRigidBody.m_bIsGrounded = false;

    // 4. Collision Integration (Swept AABB)
    // We strictly SKIP collision checks if in Fly Mode (Noclip behavior)

    // X-Axis
    if (objRigidBody.m_ObjVelocity.x != 0.0f) {
        Core::Vec3 newPos = objRigidBody.m_ObjPos;
        newPos.x += objRigidBody.m_ObjVelocity.x * fDeltaTime;

        if (!bFlyMode && checkCollision(objRigidBody.GetAABB(newPos), objChunkManager)) {
            objRigidBody.m_ObjVelocity.x = 0.0f;
        } else {
            objRigidBody.m_ObjPos.x = newPos.x;
        }
    }

    // Y-Axis
    if (objRigidBody.m_ObjVelocity.y != 0.0f) {
        Core::Vec3 newPos = objRigidBody.m_ObjPos;
        newPos.y += objRigidBody.m_ObjVelocity.y * fDeltaTime;

        if (!bFlyMode && checkCollision(objRigidBody.GetAABB(newPos), objChunkManager)) {
            if (objRigidBody.m_ObjVelocity.y < 0.0f) {
                objRigidBody.m_bIsGrounded = true;
            }
            objRigidBody.m_ObjVelocity.y = 0.0f;
        } else {
            objRigidBody.m_ObjPos.y = newPos.y;
        }
    }

    // Z-Axis
    if (objRigidBody.m_ObjVelocity.z != 0.0f) {
        Core::Vec3 newPos = objRigidBody.m_ObjPos;
        newPos.z += objRigidBody.m_ObjVelocity.z * fDeltaTime;
        if (checkCollision(objRigidBody.GetAABB(newPos), objChunkManager)) {
            objRigidBody.m_ObjVelocity.z = 0.0f;
        } else {
            objRigidBody.m_ObjPos.z = newPos.z;
        }
    }
}

//*********************************************************************
bool PhysicsSystem::checkCollision(const AABB& objAABB, const ChunkManager& objChunkManager) {
    // Determine the voxel range covering the AABB
    // Use floor to ensure we check the correct grid cells
    int iMinX = static_cast<int>(std::floor(objAABB.m_objMinPt.x));
    int iMinY = static_cast<int>(std::floor(objAABB.m_objMinPt.y));
    int iMinZ = static_cast<int>(std::floor(objAABB.m_objMinPt.z));
    int iMaxX = static_cast<int>(std::floor(objAABB.m_objMaxPt.x));
    int iMaxY = static_cast<int>(std::floor(objAABB.m_objMaxPt.y));
    int iMaxZ = static_cast<int>(std::floor(objAABB.m_objMaxPt.z));

    for (int iX = iMinX; iX <= iMaxX; iX++) {
        for (int iY = iMinY; iY <= iMaxY; iY++) {
            for (int iZ = iMinZ; iZ <= iMaxZ; iZ++) {
                int iChunkX = static_cast<int>(std::floor(static_cast<float>(iX) / CHUNK_SIZE));
                int iChunkZ = static_cast<int>(std::floor(static_cast<float>(iZ) / CHUNK_SIZE));

                const Chunk* pChunk = objChunkManager.GetChunk(iChunkX, iChunkZ);
                if (pChunk) {
                    // Convert world coord to local chunk coord
                    int iLocalX = iX - (iChunkX * CHUNK_SIZE);
                    int iLocalZ = iZ - (iChunkZ * CHUNK_SIZE);

                    if (pChunk->GetBlockAt(iLocalX, iY, iLocalZ) != 0) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}
//*********************************************************************
RayHit PhysicsSystem::RayCast(const Core::Ray& objRay,
                              float fMaxDistance,
                              const ChunkManager& objChunkManager) {
    RayHit hitResult;
    hitResult.m_bHit = false;
    hitResult.m_fDistance = fMaxDistance;

    // Initialization (DDA Setup)
    float fX = objRay.m_objPtOrigin.x;
    float fY = objRay.m_objPtOrigin.y;
    float fZ = objRay.m_objPtOrigin.z;

    int iMapX = static_cast<int>(std::floor(fX));
    int iMapY = static_cast<int>(std::floor(fY));
    int iMapZ = static_cast<int>(std::floor(fZ));

    int iStepX = (objRay.m_objDirection.x < 0) ? -1 : 1;
    int iStepY = (objRay.m_objDirection.y < 0) ? -1 : 1;
    int iStepZ = (objRay.m_objDirection.z < 0) ? -1 : 1;

    // Helper to avoid division by zero
    auto safeInv = [](float x) { return (x == 0) ? 1e30f : std::abs(1.0f / x); };

    float fDeltaX = safeInv(objRay.m_objDirection.x);
    float fDeltaY = safeInv(objRay.m_objDirection.y);
    float fDeltaZ = safeInv(objRay.m_objDirection.z);

    float fSideDistX = (iStepX < 0) ? (fX - static_cast<float>(iMapX)) * fDeltaX
                                    : (static_cast<float>(iMapX) + 1.0f - fX) * fDeltaX;
    float fSideDistY = (iStepY < 0) ? (fY - static_cast<float>(iMapY)) * fDeltaY
                                    : (static_cast<float>(iMapY) + 1.0f - fY) * fDeltaY;
    float fSideDistZ = (iStepZ < 0) ? (fZ - static_cast<float>(iMapZ)) * fDeltaZ
                                    : (static_cast<float>(iMapZ) + 1.0f - fZ) * fDeltaZ;

    int iLastAxis = 0;
    const int MAX_STEPS = 500;

    for (int i = 0; i < MAX_STEPS; i++) {
        // 1. Check current voxel
        int iChunkX = static_cast<int>(std::floor(static_cast<float>(iMapX) / CHUNK_SIZE));
        int iChunkZ = static_cast<int>(std::floor(static_cast<float>(iMapZ) / CHUNK_SIZE));
        const Chunk* pChunk = objChunkManager.GetChunk(iChunkX, iChunkZ);

        if (pChunk) {
            int iLocalX = iMapX - (iChunkX * CHUNK_SIZE);
            int iLocalZ = iMapZ - (iChunkZ * CHUNK_SIZE);
            if (pChunk->GetBlockAt(iLocalX, iMapY, iLocalZ) != 0) {
                // Hit!
                hitResult.m_bHit = true;
                hitResult.m_iBlocKX = iMapX;
                hitResult.m_iBlocKY = iMapY;
                hitResult.m_iBlocKZ = iMapZ;

                if (iLastAxis == 0)
                    hitResult.m_fDistance = fSideDistX - fDeltaX;
                else if (iLastAxis == 1)
                    hitResult.m_fDistance = fSideDistY - fDeltaY;
                else
                    hitResult.m_fDistance = fSideDistZ - fDeltaZ;

                hitResult.m_objHitPoint = objRay.at(hitResult.m_fDistance);
                return hitResult;
            }
        }

        // 2. Step
        if (fSideDistX < fSideDistY) {
            if (fSideDistX < fSideDistZ) {
                fSideDistX += fDeltaX;
                iMapX += iStepX;
                hitResult.m_objNormal = Core::Vec3(-float(iStepX), 0, 0);
                iLastAxis = 0;
            } else {
                fSideDistZ += fDeltaZ;
                iMapZ += iStepZ;
                hitResult.m_objNormal = Core::Vec3(0, 0, -float(iStepZ));
                iLastAxis = 2;
            }
        } else {
            if (fSideDistY < fSideDistZ) {
                fSideDistY += fDeltaY;
                iMapY += iStepY;
                hitResult.m_objNormal = Core::Vec3(0, -float(iStepY), 0);
                iLastAxis = 1;
            } else {
                fSideDistZ += fDeltaZ;
                iMapZ += iStepZ;
                hitResult.m_objNormal = Core::Vec3(0, 0, -float(iStepZ));
                iLastAxis = 2;
            }
        }

        // 3. Range Check
        if (std::min({fSideDistX, fSideDistY, fSideDistZ}) > fMaxDistance)
            break;
    }

    return hitResult;
}
//*********************************************************************
void PhysicsSystem::UpdateTemparature(float fDelta,
                                      int iFrameCount,
                                      ChunkManager& objChunkManager) {
    auto& mapChunks = objChunkManager.GetMutableChunks();
    for (auto& [coords, objChunk] : mapChunks) {
        objChunk.InjectHeat(8, 8, 8, 100.0f);
        objChunk.ThermalStep(0.1f, fDelta);
        if (iFrameCount == 0) {
            // objChunk.DebugPrintThermalSlice();
        }
    }
}