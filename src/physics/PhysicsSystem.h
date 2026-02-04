#pragma once

#include <algorithm>
#include <iostream>
#include <vector>
#include "../core/Ray.h"
#include "../world/Chunk.h"
#include "AABB.h"

struct RigidBody {
public:
    Core::Vec3 m_ObjPos;
    Core::Vec3 m_ObjVelocity;
    Core::Vec3 m_ObjSize;  // Half extents of (Width/2, Height/2, Length/2)

    AABB GetAABB(const Core::Vec3& newPos) const {
        return AABB(newPos - m_ObjSize, newPos + m_ObjSize);
    }
};
struct RayHit {
    Core::Vec3 m_objHitPoint{};
    Core::Vec3 m_objNormal{};
    float m_fDistance{0.0f};
    int m_iBlocKX{0}, m_iBlocKY{0}, m_iBlocKZ{0};
    bool m_bHit{false};
};
class PhysicsSystem {
public:
    static void Update(RigidBody& objRigidBody,
                       float fDeltaTime,
                       const std::vector<Chunk>& chunks) {
        const float fGravity = -20.0f;
        const float fFriction = 10.0f;

        objRigidBody.m_ObjVelocity.x =
            Lerp(objRigidBody.m_ObjVelocity.x, 0.0f, fFriction * fDeltaTime);
        objRigidBody.m_ObjVelocity.y += fGravity * fDeltaTime;
        objRigidBody.m_ObjVelocity.z =
            Lerp(objRigidBody.m_ObjVelocity.z, 0.0f, fFriction * fDeltaTime);

        if (objRigidBody.m_ObjVelocity.x != 0.0f) {
            Core::Vec3 newPos = objRigidBody.m_ObjPos;
            newPos.x += objRigidBody.m_ObjVelocity.x * fDeltaTime;
            if (CheckCollision(objRigidBody.GetAABB(newPos), chunks)) {
                objRigidBody.m_ObjVelocity.x = 0.0f;
            } else
                objRigidBody.m_ObjPos.x = newPos.x;
        }

        if (objRigidBody.m_ObjVelocity.y != 0.0f) {
            Core::Vec3 newPos = objRigidBody.m_ObjPos;
            newPos.y += objRigidBody.m_ObjVelocity.y * fDeltaTime;
            if (CheckCollision(objRigidBody.GetAABB(newPos), chunks)) {
                objRigidBody.m_ObjVelocity.y = 0.0f;
            } else
                objRigidBody.m_ObjPos.y = newPos.y;
        }

        if (objRigidBody.m_ObjVelocity.z != 0.0f) {
            Core::Vec3 newPos = objRigidBody.m_ObjPos;
            newPos.z += objRigidBody.m_ObjVelocity.z * fDeltaTime;
            if (CheckCollision(objRigidBody.GetAABB(newPos), chunks)) {
                objRigidBody.m_ObjVelocity.z = 0.0f;
            } else
                objRigidBody.m_ObjPos.z = newPos.z;
        }
    }

    // Simple Linear Interpolation for friction
    static float Lerp(float a, float b, float t) { return a + (b - a) * t; }

    static bool CheckCollision(const AABB& objAABB, const std::vector<Chunk>& chunks) {
        int iMinX = static_cast<int>(floor(objAABB.m_objMinPt.x));
        int iMinY = static_cast<int>(floor(objAABB.m_objMinPt.y));
        int iMinZ = static_cast<int>(floor(objAABB.m_objMinPt.z));
        int iMaxX = static_cast<int>(floor(objAABB.m_objMaxPt.x));
        int iMaxY = static_cast<int>(floor(objAABB.m_objMaxPt.y));
        int iMaxZ = static_cast<int>(floor(objAABB.m_objMaxPt.z));

        for (int iZ = iMinZ; iZ <= iMaxZ; iZ++) {
            for (int iY = iMinY; iY <= iMaxY; iY++) {
                for (int iX = iMinX; iX <= iMaxX; iX++) {
                    if (GetBlockAt(iX, iY, iZ, chunks) != 0) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    static uint8_t GetBlockAt(int iX, int iY, int iZ, const std::vector<Chunk>& chunks) {
        int iChunkX = static_cast<int>(floor(static_cast<float>(iX) / CHUNK_SIZE));
        int iChunkZ = static_cast<int>(floor(static_cast<float>(iZ) / CHUNK_SIZE));

        for (const auto& chunk : chunks) {
            if (chunk.GetChunkX() != iChunkX || chunk.GetChunkZ() != iChunkZ)
                continue;
            int iLocalX = iX - (iChunkX * CHUNK_SIZE);
            int iLocalZ = iZ - (iChunkZ * CHUNK_SIZE);

            return chunk.GetBlockAt(iLocalX, iY, iLocalZ);
        }
        return 0;
    }

    // DDA Algorithm for Ray Casting in Voxel Grid
    static RayHit RayCast(const Core::Ray& objRay,
                          float fMaxDistance,
                          const std::vector<Chunk>& vec_objChunks) {
        RayHit hitResult;
        hitResult.m_bHit = false;
        hitResult.m_fDistance = fMaxDistance;

        float fX = objRay.m_objPtOrigin.x;
        float fY = objRay.m_objPtOrigin.y;
        float fZ = objRay.m_objPtOrigin.z;

        int iMapX = static_cast<int>(floor(fX));
        int iMapY = static_cast<int>(floor(fY));
        int iMapZ = static_cast<int>(floor(fZ));

        int iStepX = (objRay.m_objDirection.x < 0) ? -1 : 1;
        int iStepY = (objRay.m_objDirection.y < 0) ? -1 : 1;
        int iStepZ = (objRay.m_objDirection.z < 0) ? -1 : 1;

        float fDeltaX = (objRay.m_objDirection.x == 0) ? std::numeric_limits<float>::max()
                                                       : std::abs(1.0f / objRay.m_objDirection.x);
        float fDeltaY = (objRay.m_objDirection.y == 0) ? std::numeric_limits<float>::max()
                                                       : std::abs(1.0f / objRay.m_objDirection.y);
        float fDeltaZ = (objRay.m_objDirection.z == 0) ? std::numeric_limits<float>::max()
                                                       : std::abs(1.0f / objRay.m_objDirection.z);
        float fSideDistX = (iStepX < 0) ? (fX - static_cast<float>(iMapX)) * fDeltaX
                                        : (static_cast<float>(iMapX + 1) - fX) * fDeltaX;
        float fSideDistY = (iStepY < 0) ? (fY - static_cast<float>(iMapY)) * fDeltaY
                                        : (static_cast<float>(iMapY + 1) - fY) * fDeltaY;
        float fSideDistZ = (iStepZ < 0) ? (fZ - static_cast<float>(iMapZ)) * fDeltaZ
                                        : (static_cast<float>(iMapZ + 1) - fZ) * fDeltaZ;

        int iLastAxis = 0;  // 0=X, 1=Y, 2=Z
        while (true) {
            uint8_t uiBlockType = GetBlockAt(iMapX, iMapY, iMapZ, vec_objChunks);
            if (uiBlockType != 0) {
                hitResult.m_bHit = true;
                if (iLastAxis == 0)
                    hitResult.m_fDistance = fSideDistX - fDeltaX;
                else if (iLastAxis == 1)
                    hitResult.m_fDistance = fSideDistY - fDeltaY;
                else
                    hitResult.m_fDistance = fSideDistZ - fDeltaZ;

                hitResult.m_objHitPoint = objRay.at(hitResult.m_fDistance);
                hitResult.m_iBlocKX = iMapX;
                hitResult.m_iBlocKY = iMapY;
                hitResult.m_iBlocKZ = iMapZ;

                return hitResult;
            }
            float fCurrentDistance = std::min({fSideDistX, fSideDistY, fSideDistZ});
            if (fCurrentDistance > fMaxDistance)
                break;

            if (fSideDistX < fSideDistY) {
                if (fSideDistX < fSideDistZ) {
                    iMapX += iStepX;
                    fSideDistX += fDeltaX;
                    hitResult.m_objNormal =
                        Core::Vec3((-1.0f * static_cast<float>(iStepX)), 0.0f, 0.0f);
                    iLastAxis = 0;
                } else {
                    iMapZ += iStepZ;
                    fSideDistZ += fDeltaZ;
                    hitResult.m_objNormal =
                        Core::Vec3(0.0f, 0.0f, (-1.0f * static_cast<float>(iStepZ)));
                    iLastAxis = 2;
                }
            } else {
                if (fSideDistY < fSideDistZ) {
                    iMapY += iStepY;
                    fSideDistY += fDeltaY;
                    hitResult.m_objNormal =
                        Core::Vec3(0.0f, (-1.0f * static_cast<float>(iStepY)), 0.0f);
                    iLastAxis = 1;
                } else {
                    iMapZ += iStepZ;
                    fSideDistZ += fDeltaZ;
                    hitResult.m_objNormal =
                        Core::Vec3(0.0f, 0.0f, (-1.0f * static_cast<float>(iStepZ)));
                    iLastAxis = 2;
                }
            }
        }
        return hitResult;
    }
};