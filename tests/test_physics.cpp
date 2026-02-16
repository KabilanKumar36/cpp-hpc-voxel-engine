#include <gtest/gtest.h>
#include "../src/core/MathUtils.h"
#include "../src/physics/AABB.h"

TEST(PhysicsTests, AABBCollisionDetection_True) {
    // Box 1: [0,0,0] to [1,1,1]
    Core::Vec3 minPt1(0.0f, 0.0f, 0.0f);
    Core::Vec3 maxPt1(1.0f, 1.0f, 1.0f);
    AABB box1(minPt1, maxPt1);

    // Box 2: [0.5, 0.5, 0.5] to [1.5, 1.5, 1.5] (Overlaps Box 1)
    Core::Vec3 minPt2(0.5f, 0.5f, 0.5f);
    Core::Vec3 maxPt2(1.5f, 1.5f, 1.5f);
    AABB box2(minPt2, maxPt2);

    EXPECT_TRUE(box1.CheckCollision(box2));
}

TEST(PhysicsTests, AABBCollisionDetection_False) {
    // Box 1: [0,0,0] to [1,1,1]
    Core::Vec3 minPt1(0.0f, 0.0f, 0.0f);
    Core::Vec3 maxPt1(1.0f, 1.0f, 1.0f);
    AABB box1(minPt1, maxPt1);

    // Box 2: [2,2,2] to [3,3,3] (Gap exists)
    Core::Vec3 minPt2(2.0f, 2.0f, 2.0f);
    Core::Vec3 maxPt2(3.0f, 3.0f, 3.0f);
    AABB box2(minPt2, maxPt2);

    EXPECT_FALSE(box1.CheckCollision(box2));
}

TEST(PhysicsTests, AABBInvalidConstruction) {
    Core::Vec3 minPt(10.0f, 10.0f, 10.0f);
    Core::Vec3 maxPt(0.0f, 0.0f, 0.0f);  // Error: Max < Min

// Only run this check in Debug mode where assertions are active
#ifndef NDEBUG
    EXPECT_DEATH(AABB(minPt, maxPt), ".*");
#endif
}