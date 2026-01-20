#pragma once
#include "MathUtils.h"

namespace Core {
    struct Ray {
        Vec3 origin;
        Vec3 direction;

        // Constructor
        Ray(const Vec3& orig, const Vec3& dir) 
            : origin(orig), direction(dir.normalize()) 
		{
		}

        [[nodiscard]] Vec3 at(float t) const noexcept {
            return origin + direction * t;
		}
    };
}