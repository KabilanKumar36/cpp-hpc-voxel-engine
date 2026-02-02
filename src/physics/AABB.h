#pragma once
#include <cassert>
#include "../core/Matrix.h"

class AABB {
    Core::Vec3 m_objMinPt;
    Core::Vec3 m_objMaxPt;

public:
    AABB(const Core::Vec3& ObjMinPt, const Core::Vec3& ObjMaxPt)
        : m_objMinPt(ObjMinPt), m_objMaxPt(ObjMaxPt) {
        assert(m_objMinPt.x <= m_objMaxPt.x && "Min. x must be less than or equal to Max. x");
        assert(m_objMinPt.y <= m_objMaxPt.y && "Min. y must be less than or equal to Max. y");
        assert(m_objMinPt.z <= m_objMaxPt.z && "Min. z must be less than or equal to Max. z");
    }
    bool CheckCollision(const AABB& Other) const {
        if (m_objMaxPt.x < Other.m_objMinPt.x || m_objMinPt.x > Other.m_objMaxPt.x)
            return false;
        if (m_objMaxPt.y < Other.m_objMinPt.y || m_objMinPt.y > Other.m_objMaxPt.y)
            return false;
        if (m_objMaxPt.z < Other.m_objMinPt.z || m_objMinPt.z > Other.m_objMaxPt.z)
            return false;
        return true;
        /*return ((m_objMaxPt.x >= Other.m_objMinPt.x && m_objMinPt.x <= Other.m_objMaxPt.x) &&
            (m_objMaxPt.y >= Other.m_objMinPt.y && m_objMinPt.y <= Other.m_objMaxPt.y) &&
            (m_objMaxPt.z >= Other.m_objMinPt.z && m_objMinPt.z <= Other.m_objMaxPt.z));*/
    }

    Core::Vec3 GetCenter() const { return (m_objMinPt + m_objMaxPt) * 0.5f; }
};