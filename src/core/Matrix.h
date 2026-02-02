#pragma once
#include <cmath>
#include <cstring>
#include "MathUtils.h"

namespace Core {
struct Mat4 {
    float elements[16];  // Column-major order (OpenGL Standard)

    // 1. Constructor: Identity Matrix
    constexpr Mat4() noexcept {
        for (int i = 0; i < 16; i++) elements[i] = 0.0f;
        // Diagonal = 1.0
        elements[0 + 0 * 4] = 1.0f;
        elements[1 + 1 * 4] = 1.0f;
        elements[2 + 2 * 4] = 1.0f;
        elements[3 + 3 * 4] = 1.0f;
    }

    static Mat4 identity() { return Mat4(); }

    static Mat4 zero() {
        Mat4 result;
        // Manual loop for constexpr compatibility, or memset if not constexpr
        for (int i = 0; i < 16; i++) result.elements[i] = 0.0f;
        return result;
    }

    // 2. Operation: Matrix Multiplication
    // FIXED: Corrected to Column-Major indexing [row + col * 4]
    Mat4 operator*(const Mat4& other) const {
        Mat4 result = Mat4::zero();
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    // Row of A * Column of B
                    sum += elements[row + k * 4] * other.elements[k + col * 4];
                }
                result.elements[row + col * 4] = sum;
            }
        }
        return result;
    }

    // 3. Camera: Perspective Projection
    // FIXED: Swapped indices 11 and 14 to correct spot
    static Mat4 perspective(float fov, float aspect, float nearplane, float farplane) {
        Mat4 result = Mat4::zero();
        float tanHalfFOV = std::tan(fov * 3.1415926535f * 0.5f / 180.0f);

        result.elements[0 + 0 * 4] = 1.0f / (aspect * tanHalfFOV);
        result.elements[1 + 1 * 4] = 1.0f / (tanHalfFOV);
        result.elements[2 + 2 * 4] = -(farplane + nearplane) / (farplane - nearplane);

        // Critical Fixes:
        result.elements[2 + 3 * 4] = -1.0f;  // Index 11 (Row 3, Col 2): Perspective division
        result.elements[3 + 2 * 4] = 0.0f;   // Clear this spot
        result.elements[3 + 2 * 4] =
            -(2.0f * farplane * nearplane) /
            (farplane - nearplane);  // Wait, index is [14] (Row 2, Col 3) in column major?

        // Let's be explicit to avoid confusion:
        // Col 2 (Z-axis interaction)
        result.elements[10] = -(farplane + nearplane) / (farplane - nearplane);
        result.elements[11] = -1.0f;

        // Col 3 (Translation)
        result.elements[14] = -(2.0f * farplane * nearplane) / (farplane - nearplane);
        result.elements[15] = 0.0f;

        return result;
    }

    // 4. Camera: LookAt
    // FIXED: Corrected W component and rotation layout
    static Mat4 lookAt(const Vec3& eye, const Vec3& tgt, const Vec3& up) {
        Mat4 result = Mat4::identity();
        Vec3 fwd = (tgt - eye).normalize();
        Vec3 right = fwd.cross(up).normalize();
        Vec3 trueUp = right.cross(fwd);

        // Rotation (Transposed)
        // Column 0 (Right)
        result.elements[0] = right.x;
        result.elements[4] = right.y;
        result.elements[8] = right.z;

        // Column 1 (Up)
        result.elements[1] = trueUp.x;
        result.elements[5] = trueUp.y;
        result.elements[9] = trueUp.z;

        // Column 2 (Forward - Negated)
        result.elements[2] = -fwd.x;
        result.elements[6] = -fwd.y;
        result.elements[10] = -fwd.z;

        // Column 3 (Translation)
        result.elements[12] = -right.dot(eye);
        result.elements[13] = -trueUp.dot(eye);
        result.elements[14] = fwd.dot(eye);  // Positive dot because Z is inverted
        result.elements[15] = 1.0f;          // FIXED: Must be 1.0f, not -1.0f

        return result;
    }
    static Mat4 orthographic(
        float left, float right, float bottom, float top, float nearplane, float farplane) {
        Mat4 result;

        result.elements[0 + 0 * 4] = 2.0f / (right - left);
        result.elements[1 + 1 * 4] = 2.0f / (top - bottom);
        result.elements[2 + 2 * 4] = -2.0f / (farplane - nearplane);

        result.elements[0 + 3 * 4] = -(right + left) / (right - left);
        result.elements[1 + 3 * 4] = -(top + bottom) / (top - bottom);
        result.elements[2 + 3 * 4] = -(farplane + nearplane) / (farplane - nearplane);

        return result;
    }

    static Mat4 Translation(const Vec3& translation) {
        Mat4 result;
        result.elements[0 + 3 * 4] = translation.x;
        result.elements[1 + 3 * 4] = translation.y;
        result.elements[2 + 3 * 4] = translation.z;
        return result;
    }

    static Mat4 Scale(float scaleX, float scaleY, float scaleZ) {
        Mat4 result;
        result.elements[0 + 0 * 4] = scaleX;
        result.elements[1 + 1 * 4] = scaleY;
        result.elements[2 + 2 * 4] = scaleZ;
        return result;
    }
};
}  // namespace Core