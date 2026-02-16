#pragma once
#include <cmath>
#include <cstring>
#include "MathUtils.h"

namespace Core {

/**
 * @struct Mat4
 * @brief A 4x4 Matrix structure stored in Column-Major order (OpenGL Standard).
 *
 * Index Layout:
 * 0  4  8 12
 * 1  5  9 13
 * 2  6 10 14
 * 3  7 11 15
 */
struct Mat4 {
    float m_fElements[16];

    // --- Constructors ---

    /**
     * @brief Default constructor initializes to Identity Matrix.
     */
    constexpr Mat4() noexcept {
        // Initialize all to 0.0f
        for (int i = 0; i < 16; i++) m_fElements[i] = 0.0f;
        // Diagonal = 1.0f (Identity)
        m_fElements[0] = 1.0f;
        m_fElements[5] = 1.0f;
        m_fElements[10] = 1.0f;
        m_fElements[15] = 1.0f;
    }

    // Helper to get Identity (matches default constructor)
    static Mat4 Identity() { return Mat4(); }

    // Helper to get a pure Zero matrix (for Projections)
    static Mat4 Zero() {
        Mat4 result;
        // We must manually zero it out because the constructor sets diagonals to 1
        for (int i = 0; i < 16; i++) result.m_fElements[i] = 0.0f;
        return result;
    }

    // --- Operations ---

    /**
     * @brief Matrix Multiplication.
     * @return New Matrix result = (*this) * other
     */
    Mat4 operator*(const Mat4& other) const {
        Mat4 result = Mat4::Zero();
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    // Row of A * Column of B
                    sum += m_fElements[row + k * 4] * other.m_fElements[k + col * 4];
                }
                result.m_fElements[row + col * 4] = sum;
            }
        }
        return result;
    }

    // --- Projections & Transforms ---

    /**
     * @brief Creates a Perspective Projection Matrix (FOV Y).
     * @param fov Field of View in degrees.
     * @param aspect Aspect Ratio (Width / Height).
     * @param nearplane Distance to near clipping plane.
     * @param farplane Distance to far clipping plane.
     */
    static Mat4 Perspective(float fov, float aspect, float nearplane, float farplane) {
        Mat4 result = Mat4::Zero();
        float tanHalfFOV = std::tan(fov * 3.1415926535f * 0.5f / 180.0f);

        // [0][0] = 1 / (aspect * tan(fov/2))
        result.m_fElements[0] = 1.0f / (aspect * tanHalfFOV);

        // [1][1] = 1 / tan(fov/2)
        result.m_fElements[5] = 1.0f / (tanHalfFOV);

        // [2][2] = -(far + near) / (far - near)
        result.m_fElements[10] = -(farplane + nearplane) / (farplane - nearplane);

        // [2][3] = -1 (This is Index 11 in Column-Major: Row 3, Col 2)
        // This puts -Z into W for perspective divide
        result.m_fElements[11] = -1.0f;

        // [3][2] = -(2 * far * near) / (far - near) (Index 14: Row 2, Col 3)
        result.m_fElements[14] = -(2.0f * farplane * nearplane) / (farplane - nearplane);

        // [3][3] = 0
        result.m_fElements[15] = 0.0f;

        return result;
    }

    /**
     * @brief Creates a View Matrix using Eye, Target, and Up vectors.
     */
    static Mat4 LookAt(const Vec3& eye, const Vec3& tgt, const Vec3& up) {
        Mat4 result;
        Vec3 fwd = (tgt - eye).normalize();
        Vec3 right = fwd.cross(up).normalize();
        Vec3 trueUp = right.cross(fwd);

        // Rotation Part (Orthonormal Basis)
        // Row 0: Right
        result.m_fElements[0] = right.x;
        result.m_fElements[4] = right.y;
        result.m_fElements[8] = right.z;

        // Row 1: Up
        result.m_fElements[1] = trueUp.x;
        result.m_fElements[5] = trueUp.y;
        result.m_fElements[9] = trueUp.z;

        // Row 2: -Forward (Looking down -Z)
        result.m_fElements[2] = -fwd.x;
        result.m_fElements[6] = -fwd.y;
        result.m_fElements[10] = -fwd.z;

        // Translation Part (Dot products)
        result.m_fElements[12] = -right.dot(eye);
        result.m_fElements[13] = -trueUp.dot(eye);
        result.m_fElements[14] = fwd.dot(eye);  // Dot with positive fwd because Z is negated

        return result;
    }

    static Mat4 Orthographic(
        float left, float right, float bottom, float top, float nearplane, float farplane) {
        Mat4 result;

        result.m_fElements[0] = 2.0f / (right - left);
        result.m_fElements[5] = 2.0f / (top - bottom);
        result.m_fElements[10] = -2.0f / (farplane - nearplane);

        result.m_fElements[12] = -(right + left) / (right - left);
        result.m_fElements[13] = -(top + bottom) / (top - bottom);
        result.m_fElements[14] = -(farplane + nearplane) / (farplane - nearplane);

        return result;
    }

    static Mat4 Translation(const Vec3& translation) {
        Mat4 result;
        result.m_fElements[12] = translation.x;
        result.m_fElements[13] = translation.y;
        result.m_fElements[14] = translation.z;
        return result;
    }

    static Mat4 Scale(float scaleX, float scaleY, float scaleZ) {
        Mat4 result;
        result.m_fElements[0] = scaleX;
        result.m_fElements[5] = scaleY;
        result.m_fElements[10] = scaleZ;
        return result;
    }
};
}  // namespace Core