#pragma once
#include "MathUtils.h"

namespace Core {

/**
 * @struct Ray
 * @brief Represents a ray in 3D space defined by an origin and a direction.
 * Used for raycasting and picking.
 */
struct Ray {
    Vec3 m_objPtOrigin;
    Vec3 m_objDirection;

    /**
     * @brief Constructs a ray.
     * @param orig The starting point of the ray.
     * @param dir The direction vector (automatically normalized).
     */
    Ray(const Vec3& orig, const Vec3& dir) : m_objPtOrigin(orig), m_objDirection(dir.normalize()) {}

    /**
     * @brief Calculates a point along the ray at distance t.
     * @param t Distance from origin.
     * @return Point = Origin + (Direction * t)
     */
    [[nodiscard]] Vec3 at(float t) const noexcept { return m_objPtOrigin + (m_objDirection * t); }
};
}  // namespace Core