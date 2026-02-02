#pragma once
#include "MathUtils.h"

namespace Core {
struct Ray {
    Vec3 m_objPtOrigin;
    Vec3 m_objDirection;

    // Constructor
    Ray(const Vec3& orig, const Vec3& dir) : m_objPtOrigin(orig), m_objDirection(dir.normalize()) {}

    [[nodiscard]] Vec3 at(float t) const noexcept { return m_objPtOrigin + (m_objDirection * t); }
};
}  // namespace Core