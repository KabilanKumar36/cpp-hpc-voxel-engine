#include <gtest/gtest.h>
#include "../src/physics/AABB.h"

TEST(PhysicsTests, AABBCollisionDetection_true) {
	Core::Vec3 minPt1(0.0f, 0.0f, 0.0f);
	Core::Vec3 maxPt1(1.0f, 1.0f, 1.0f);
	AABB box1(minPt1, maxPt1);

	Core::Vec3 minPt2(0.5f, 0.5f, 0.5f);
	Core::Vec3 maxPt2(1.5f, 1.5f, 1.5f);
	AABB box2(minPt2, maxPt2);

	EXPECT_TRUE(box1.CheckCollision(box2));
}

TEST(PhysicsTests, AABBCollisionDetection_false) {
	Core::Vec3 minPt1(0.0f, 0.0f, 0.0f);
	Core::Vec3 maxPt1(1.0f, 1.0f, 1.0f);
	AABB box1(minPt1, maxPt1);

	Core::Vec3 minPt2(2.0f, 2.0f, 2.0f);
	Core::Vec3 maxPt2(3.0f, 3.0f, 3.0f);
	AABB box2(minPt2, maxPt2);

	EXPECT_FALSE(box1.CheckCollision(box2));
}

TEST(PhysicsTests, AABBInvalidConstruction) {
    Core::Vec3 minPt(10.0f, 10.0f, 10.0f);
    Core::Vec3 maxPt(0.0f, 0.0f, 0.0f); // Max is smaller than Min!
    
    #ifndef NDEBUG 
    EXPECT_DEATH(AABB(minPt, maxPt), ".*");
    #endif
}