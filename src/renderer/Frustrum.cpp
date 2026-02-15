#include "Frustrum.h"

// ********************************************************************
enum PlaneSide { PLANE_LEFT = 0, PLANE_RIGHT, PLANE_TOP, PLANE_BOTTOM, PLANE_NEAR, PLANE_FAR };
// ********************************************************************
void Frustrum::Update(const Core::Mat4& viewProjMatrix) {
    // Gribb-Hartmann plane extraction method, adapted for column-major order
    //  Left plane
    m_arrPlanes[PLANE_LEFT] = {viewProjMatrix.elements[3] + viewProjMatrix.elements[0],
                               viewProjMatrix.elements[7] + viewProjMatrix.elements[4],
                               viewProjMatrix.elements[11] + viewProjMatrix.elements[8],
                               viewProjMatrix.elements[15] + viewProjMatrix.elements[12]};

    // Right plane
    m_arrPlanes[PLANE_RIGHT] = {viewProjMatrix.elements[3] - viewProjMatrix.elements[0],
                                viewProjMatrix.elements[7] - viewProjMatrix.elements[4],
                                viewProjMatrix.elements[11] - viewProjMatrix.elements[8],
                                viewProjMatrix.elements[15] - viewProjMatrix.elements[12]};

    // Top plane
    m_arrPlanes[PLANE_TOP] = {viewProjMatrix.elements[3] - viewProjMatrix.elements[1],
                              viewProjMatrix.elements[7] - viewProjMatrix.elements[5],
                              viewProjMatrix.elements[11] - viewProjMatrix.elements[9],
                              viewProjMatrix.elements[15] - viewProjMatrix.elements[13]};

    // Bottom plane
    m_arrPlanes[PLANE_BOTTOM] = {viewProjMatrix.elements[3] + viewProjMatrix.elements[1],
                                 viewProjMatrix.elements[7] + viewProjMatrix.elements[5],
                                 viewProjMatrix.elements[11] + viewProjMatrix.elements[9],
                                 viewProjMatrix.elements[15] + viewProjMatrix.elements[13]};

    // Near plane
    m_arrPlanes[PLANE_NEAR] = {viewProjMatrix.elements[3] + viewProjMatrix.elements[2],
                               viewProjMatrix.elements[7] + viewProjMatrix.elements[6],
                               viewProjMatrix.elements[11] + viewProjMatrix.elements[10],
                               viewProjMatrix.elements[15] + viewProjMatrix.elements[14]};

    // Far plane
    m_arrPlanes[PLANE_FAR] = {viewProjMatrix.elements[3] - viewProjMatrix.elements[2],
                              viewProjMatrix.elements[7] - viewProjMatrix.elements[6],
                              viewProjMatrix.elements[11] - viewProjMatrix.elements[10],
                              viewProjMatrix.elements[15] - viewProjMatrix.elements[14]};

    // Normalize the planes
    for (auto& plane : m_arrPlanes) {
        plane.Normalize();
    }
}
// ********************************************************************
bool Frustrum::IsBoxInVisibleFrustrum(const AABB& box) const {
    for (const auto& plane : m_arrPlanes) {
        // Check if all corners of the AABB are outside the plane
        if (plane.getSignedDistanceToPlane(box.m_objMinPt) < 0 &&
            plane.getSignedDistanceToPlane({box.m_objMaxPt.x, box.m_objMinPt.y, box.m_objMinPt.z}) <
                0 &&
            plane.getSignedDistanceToPlane({box.m_objMinPt.x, box.m_objMaxPt.y, box.m_objMinPt.z}) <
                0 &&
            plane.getSignedDistanceToPlane({box.m_objMaxPt.x, box.m_objMaxPt.y, box.m_objMinPt.z}) <
                0 &&
            plane.getSignedDistanceToPlane({box.m_objMinPt.x, box.m_objMinPt.y, box.m_objMaxPt.z}) <
                0 &&
            plane.getSignedDistanceToPlane({box.m_objMaxPt.x, box.m_objMinPt.y, box.m_objMaxPt.z}) <
                0 &&
            plane.getSignedDistanceToPlane({box.m_objMinPt.x, box.m_objMaxPt.y, box.m_objMaxPt.z}) <
                0 &&
            plane.getSignedDistanceToPlane(box.m_objMaxPt) < 0) {
            return false;  // Box is completely outside this plane
        }
    }
    return true;  // Box is at least partially inside the frustum
}
// ********************************************************************