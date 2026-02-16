#pragma once

#include <algorithm>
#include <cassert>
#include "../core/MathUtils.h"

/**
 * @class AABB
 * @brief Axis-Aligned Bounding Box for collision detection.
 * Defined by a Minimum and Maximum point in 3D space.
 */
class AABB {
public:
    Core::Vec3 m_objMinPt;
    Core::Vec3 m_objMaxPt;

    /**
     * @brief Constructs an AABB from two points.
     * @note Enforces that Min <= Max. If this triggers, check your RigidBody size!
     */
    AABB(const Core::Vec3& ObjMinPt, const Core::Vec3& ObjMaxPt)
        : m_objMinPt(ObjMinPt), m_objMaxPt(ObjMaxPt) {
        // Strict Validation: Stop execution if data is invalid.
        // If these trigger, you likely have a negative size in your RigidBody.
        assert(m_objMinPt.x <= m_objMaxPt.x && "AABB Error: Min.X > Max.X");
        assert(m_objMinPt.y <= m_objMaxPt.y && "AABB Error: Min.Y > Max.Y");
        assert(m_objMinPt.z <= m_objMaxPt.z && "AABB Error: Min.Z > Max.Z");
    }

    /**
     * @brief Checks intersection with another AABB.
     * @return True if overlapping, False otherwise.
     */
    bool CheckCollision(const AABB& Other) const {
        // Exit early if there is a gap on any axis
        if (m_objMaxPt.x < Other.m_objMinPt.x || m_objMinPt.x > Other.m_objMaxPt.x)
            return false;
        if (m_objMaxPt.y < Other.m_objMinPt.y || m_objMinPt.y > Other.m_objMaxPt.y)
            return false;
        if (m_objMaxPt.z < Other.m_objMinPt.z || m_objMinPt.z > Other.m_objMaxPt.z)
            return false;

        return true;
    }

    Core::Vec3 GetCenter() const { return (m_objMinPt + m_objMaxPt) * 0.5f; }
    Core::Vec3 GetHalfExtents() const { return (m_objMaxPt - m_objMinPt) * 0.5f; }

    void Translate(const Core::Vec3& offset) {
        m_objMinPt += offset;
        m_objMaxPt += offset;
    }
};