#pragma once
#include <cmath>
#include <iostream>

namespace Core {
struct Vec3 {
    float x, y, z;
    // Constructors
    constexpr Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    constexpr Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    // Operator Overloads
    [[nodiscard]] constexpr Vec3 operator+(const Vec3& other) const noexcept {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }
    [[nodiscard]] constexpr Vec3 operator-(const Vec3& other) const noexcept {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }
    constexpr Vec3& operator+=(const Vec3& other) noexcept {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
    constexpr Vec3& operator-=(const Vec3& other) noexcept {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }
    [[nodiscard]] constexpr Vec3 operator*(float scalar) const noexcept {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }
    [[nodiscard]] constexpr Vec3 operator/(float scalar) const noexcept {
        return Vec3(x / scalar, y / scalar, z / scalar);
    }
    // Utility Functions
    [[nodiscard]] float squaredLength() const { return x * x + y * y + z * z; }
    [[nodiscard]] float length() const { return std::sqrt(x * x + y * y + z * z); }
    [[nodiscard]] Vec3 normalize() const {
        float len = length();
        if (len < 1e-5f)
            return Vec3(0.0f, 0.0f, 0.0f);
        return Vec3(x / len, y / len, z / len);
    }
    [[nodiscard]] float SquaredDistanceFromPoint(const Core::Vec3& other) const {
        Vec3 diff = *this - other;
        return diff.squaredLength();
    }
    [[nodiscard]] float DistanceFromPoint(const Core::Vec3& other) const {
        Vec3 diff = *this - other;
        return diff.length();
    }

    // Dot and Cross Product
    [[nodiscard]] constexpr float dot(const Vec3& other) const noexcept {
        return x * other.x + y * other.y + z * other.z;
    }
    [[nodiscard]] constexpr Vec3 cross(const Vec3& other) const noexcept {
        return Vec3(
            y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
    }

    void print() const { std::cout << "Vec3(" << x << ", " << y << ", " << z << ")\n"; }
    // Debug Helper
    friend std::ostream& operator<<(std::ostream& os, const Vec3& vec) {
        os << "Vec3(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
        return os;
    }
};
}  // namespace Core