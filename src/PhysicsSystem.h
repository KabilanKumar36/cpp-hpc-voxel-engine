#include "Core/AABB.h"
#include <vector>
#include <iostream>

struct RigidBody {
public:
	Core::Vec3 m_ObjPos;
	Core::Vec3 m_ObjVelocity;
	Core::Vec3 m_ObjSize; //Half extents of (Width/2, Height/2, Length/2)

	AABB GetAABB(const Core::Vec3 &newPos) const {
		return AABB(newPos - m_ObjSize, newPos + m_ObjSize);
	}

};

class PhysicsSystem {
public:
	void Update(RigidBody &objBody, float fDelTime, const std::vector<AABB>& vec_objObstacles) {
		const float fGravity = -9.81f;
		objBody.m_ObjVelocity.y += fGravity * fDelTime;

		if (objBody.m_ObjVelocity.x != 0.0f) {
			Core::Vec3 newPos = objBody.m_ObjPos;
			newPos.x += objBody.m_ObjVelocity.x * fDelTime;
			if (!CheckCollision(objBody.GetAABB(newPos), vec_objObstacles)) {
				objBody.m_ObjPos.x = newPos.x;
			}
			else {
				objBody.m_ObjVelocity.x = 0.0f;
			}
		}

		if (objBody.m_ObjVelocity.y != 0.0f) {
			Core::Vec3 newPos = objBody.m_ObjPos;
			newPos.y += objBody.m_ObjVelocity.y * fDelTime;
			if (!CheckCollision(objBody.GetAABB(newPos), vec_objObstacles)) {
				objBody.m_ObjPos.y = newPos.y;
			}
			else {
				objBody.m_ObjVelocity.y = 0.0f;
			}
		}

		if (objBody.m_ObjVelocity.z != 0.0f) {
			Core::Vec3 newPos = objBody.m_ObjPos;
			newPos.z += objBody.m_ObjVelocity.z * fDelTime;
			if (!CheckCollision(objBody.GetAABB(newPos), vec_objObstacles)) {
				objBody.m_ObjPos.z = newPos.z;
			}
			else {
				objBody.m_ObjVelocity.z = 0.0f;
			}
		}

	}
	bool CheckCollision(const AABB& objAABB, const std::vector<AABB>& vec_objObstacles) {
		for (const auto& obstacle : vec_objObstacles) {
			if (objAABB.CheckCollision(obstacle))
				return true;
		}
		return false;
	}

};