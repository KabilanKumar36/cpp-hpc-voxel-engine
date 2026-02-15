#pragma once

#include <array>
#include <cmath>
#include "../core/MathUtils.h"
#include "../physics/AABB.h"

struct Plane {
    float a = 0.0f, b = 0.0f, c = 0.0f, d = 0.0f;

    Plane() = default;
    Plane(float _a, float _b, float _c, float _d) : a(_a), b(_b), c(_c), d(_d) {}
    Plane(const Core::Vec3& n, float dist) : a(n.x), b(n.y), c(n.z), d(dist) {}
    void Normalize() {
        float length = std::sqrt(a * a + b * b + c * c);
        if (length > 0.0f) {
            a /= length;
            b /= length;
            c /= length;
            d /= length;
        }
    }
    float getSignedDistanceToPlane(const Core::Vec3& point) const {
        return a * point.x + b * point.y + c * point.z + d;
    }
};

class Frustrum {
public:
    Frustrum() = default;
    void Update(const Core::Mat4& viewProjMatrix);
    bool IsBoxInVisibleFrustrum(const AABB& box) const;

private:
    std::array<Plane, 6> m_arrPlanes;
};
