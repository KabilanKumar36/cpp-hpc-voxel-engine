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
    Core::Vec3 m_objHitPoint;
    Core::Vec3 m_objNormal;
    float m_fDistance;
    uint8_t m_iBlocKX, m_iBlocKY, m_iBlocKZ;
    bool m_bHit;
};
class PhysicsSystem {
public:
    void Update(RigidBody& objBody, float fDelTime, const std::vector<AABB>& vec_objObstacles) {
        const float fGravity = -9.81f;
        objBody.m_ObjVelocity.y += fGravity * fDelTime;

        if (objBody.m_ObjVelocity.x != 0.0f) {
            Core::Vec3 newPos = objBody.m_ObjPos;
            newPos.x += objBody.m_ObjVelocity.x * fDelTime;
            if (!CheckCollision(objBody.GetAABB(newPos), vec_objObstacles)) {
                objBody.m_ObjPos.x = newPos.x;
            } else {
                objBody.m_ObjVelocity.x = 0.0f;
            }
        }

        if (objBody.m_ObjVelocity.y != 0.0f) {
            Core::Vec3 newPos = objBody.m_ObjPos;
            newPos.y += objBody.m_ObjVelocity.y * fDelTime;
            if (!CheckCollision(objBody.GetAABB(newPos), vec_objObstacles)) {
                objBody.m_ObjPos.y = newPos.y;
            } else {
                objBody.m_ObjVelocity.y = 0.0f;
            }
        }

        if (objBody.m_ObjVelocity.z != 0.0f) {
            Core::Vec3 newPos = objBody.m_ObjPos;
            newPos.z += objBody.m_ObjVelocity.z * fDelTime;
            if (!CheckCollision(objBody.GetAABB(newPos), vec_objObstacles)) {
                objBody.m_ObjPos.z = newPos.z;
            } else {
                objBody.m_ObjVelocity.z = 0.0f;
            }
        }
    }
    bool CheckCollision(const AABB& objAABB, const std::vector<AABB>& vec_objObstacles) {
        for (const auto& obstacle : vec_objObstacles) {
            if (objAABB.CheckCollision(obstacle))
                return true;
        }
        return false;
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

        while (true) {
            int iChunkX = static_cast<int>(floor(static_cast<float>(iMapX) / CHUNK_SIZE));
            int iChunkZ = static_cast<int>(floor(static_cast<float>(iMapZ) / CHUNK_SIZE));
            for (const auto& chunk : vec_objChunks) {
                if (chunk.GetChunkX() != iChunkX || chunk.GetChunkZ() != iChunkZ)
                    continue;
                uint8_t uiBlockType = chunk.GetBlockAt(
                    iMapX - iChunkX * CHUNK_SIZE, iMapY, iMapZ - iChunkZ * CHUNK_SIZE);

                if (!uiBlockType)
                    break;  // Air Block, continue ray marching

                hitResult.m_bHit = true;
                if (hitResult.m_objHitPoint.x != 0)
                    hitResult.m_fDistance = fSideDistX - fDeltaX;
                else if (hitResult.m_objHitPoint.y != 0)
                    hitResult.m_fDistance = fSideDistY - fDeltaY;
                else
                    hitResult.m_fDistance = fSideDistZ - fDeltaZ;

                hitResult.m_objHitPoint = objRay.at(hitResult.m_fDistance);
                hitResult.m_iBlocKX = static_cast<uint8_t>(iMapX);
                hitResult.m_iBlocKY = static_cast<uint8_t>(iMapY);
                hitResult.m_iBlocKZ = static_cast<uint8_t>(iMapZ);

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
                } else {
                    iMapZ += iStepZ;
                    fSideDistZ += fDeltaZ;
                    hitResult.m_objNormal =
                        Core::Vec3(0.0f, 0.0f, (-1.0f * static_cast<float>(iStepZ)));
                }
            } else {
                if (fSideDistY < fSideDistZ) {
                    iMapY += iStepY;
                    fSideDistY += fDeltaY;
                    hitResult.m_objNormal =
                        Core::Vec3(0.0f, (-1.0f * static_cast<float>(iStepY)), 0.0f);
                } else {
                    iMapZ += iStepZ;
                    fSideDistZ += fDeltaZ;
                    hitResult.m_objNormal =
                        Core::Vec3(0.0f, 0.0f, (-1.0f * static_cast<float>(iStepZ)));
                }
            }
        }
        return hitResult;
    }
};