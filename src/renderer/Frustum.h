#pragma once

#include <array>
#include <cmath>
#include "../core/MathUtils.h"
#include "../core/Matrix.h"
#include "../physics/AABB.h"

struct Plane {
    float a = 0.0f, b = 0.0f, c = 0.0f, d = 0.0f;

    Plane() = default;
    Plane(float _a, float _b, float _c, float _d) : a(_a), b(_b), c(_c), d(_d) {}

    void Normalize() {
        float length = std::sqrt(a * a + b * b + c * c);
        if (length > 0.0f) {
            float invLength = 1.0f / length;
            a *= invLength;
            b *= invLength;
            c *= invLength;
            d *= invLength;
        }
    }

    float GetSignedDistanceToPlane(const Core::Vec3& point) const {
        return a * point.x + b * point.y + c * point.z + d;
    }
};

/**
 * @class Frustum
 * @brief Represents the camera's viewing volume. Used for Culling.
 */
class Frustum {
public:
    Frustum() = default;

    /**
     * @brief Extracts the 6 frustum planes from the View-Projection matrix.
     * Uses the Gribb-Hartmann method.
     */
    void Update(const Core::Mat4& viewProjMatrix);

    /**
     * @brief Checks if an AABB is visible within the frustum.
     * Uses the "Center + Extent" method for high performance.
     */
    bool IsBoxInVisibleFrustum(const AABB& box) const;

private:
    std::array<Plane, 6> m_arrPlanes;
};