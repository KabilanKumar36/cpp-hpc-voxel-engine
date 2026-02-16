#include "Frustum.h"

// Enum for readable array access
enum PlaneSide { PLANE_LEFT = 0, PLANE_RIGHT, PLANE_TOP, PLANE_BOTTOM, PLANE_NEAR, PLANE_FAR };
// ********************************************************************
void Frustum::Update(const Core::Mat4& viewProjMatrix) {
    const float* e = viewProjMatrix.m_fElements;

    // Gribb-Hartmann plane extraction
    // Left plane:   Row 4 + Row 1
    m_arrPlanes[PLANE_LEFT] = {e[3] + e[0], e[7] + e[4], e[11] + e[8], e[15] + e[12]};

    // Right plane:  Row 4 - Row 1
    m_arrPlanes[PLANE_RIGHT] = {e[3] - e[0], e[7] - e[4], e[11] - e[8], e[15] - e[12]};

    // Top plane:    Row 4 - Row 2
    m_arrPlanes[PLANE_TOP] = {e[3] - e[1], e[7] - e[5], e[11] - e[9], e[15] - e[13]};

    // Bottom plane: Row 4 + Row 2
    m_arrPlanes[PLANE_BOTTOM] = {e[3] + e[1], e[7] + e[5], e[11] + e[9], e[15] + e[13]};

    // Near plane:   Row 4 + Row 3
    m_arrPlanes[PLANE_NEAR] = {e[3] + e[2], e[7] + e[6], e[11] + e[10], e[15] + e[14]};

    // Far plane:    Row 4 - Row 3
    m_arrPlanes[PLANE_FAR] = {e[3] - e[2], e[7] - e[6], e[11] - e[10], e[15] - e[14]};

    for (auto& plane : m_arrPlanes) {
        plane.Normalize();
    }
}
// ********************************************************************
bool Frustum::IsBoxInVisibleFrustum(const AABB& box) const {
    Core::Vec3 center = box.GetCenter();
    Core::Vec3 extents = box.GetHalfExtents();

    for (const auto& plane : m_arrPlanes) {
        // Compute the projection interval radius of the AABB onto the plane normal
        float r = extents.x * std::abs(plane.a) + extents.y * std::abs(plane.b) +
                  extents.z * std::abs(plane.c);

        float distance = plane.GetSignedDistanceToPlane(center);

        // If center is behind the plane by more than the radius, it's outside
        if (distance < -r) {
            return false;
        }
    }
    return true;
}