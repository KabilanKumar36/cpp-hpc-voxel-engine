#pragma once
#include "MathUtils.h"
#include "Matrix.h"
#include <cmath>

namespace Core {
	constexpr float PI = 3.1415926535f;
	constexpr float YAW = -90.0f;
	constexpr float PITCH = 0.0f;
	constexpr float SPEED = 1.0f;
	constexpr float SENSITIVITY = 0.001f;
	constexpr float ZOOM = 45.0f;
	class Camera {
	public:
		Vec3 position;
		Vec3 front;
		Vec3 up;
		Vec3 right;
		Vec3 worldUp;

		//Euler Angles
		float yaw;
		float pitch;


		//Constructor
		Camera(Vec3 startPos = Vec3(0.0f, 0.0f, 0.0f), Vec3 startUP = Vec3(0.0f, 1.0f, 0.0f))
			: front(Vec3(0.0f, 0.0f, -1.0f)), speed(SPEED), sensitivity(SENSITIVITY), zoom(ZOOM)
		{
			position = startPos;
			worldUp = startUP;
			yaw = YAW;
			pitch = PITCH;
			updateCameraVectors();
		}

		Mat4 getViewMatrix() {
			return Mat4::lookAt(position, position + front, up);
		}

		void processKeyboard(int iDirection, float fdeltaTime)
		{
			float velocity = speed * fdeltaTime;
			if (iDirection == 0) //FORWARD
				position = position + front * velocity;
			if (iDirection == 1) //BACKWARD
				position = position - front * velocity;
			if (iDirection == 2) //LEFT
				position = position - right * velocity;
			if (iDirection == 3) //RIGHT
				position = position + right * velocity;
			if (iDirection == 4) //UP
				position = position + up * velocity;
			if (iDirection == 5) //DOWN
				position = position - up * velocity;
		}

		void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true)
		{
			xOffset *= sensitivity;
			yOffset *= sensitivity;
			yaw += xOffset;
			pitch += yOffset;
			if (constrainPitch)
			{
				if (pitch > 89.0f)
					pitch = 89.0f;
				if (pitch < -89.0f)
					pitch = -89.0f;
			}
			updateCameraVectors();
		}
	private:

		float speed;
		float sensitivity;
		float zoom;

		void updateCameraVectors() {
			Vec3 fwd;
			fwd.x = std::cos(yaw * PI / 180.0f) * std::cos(pitch * PI / 180.0f);
			fwd.y = std::sin(pitch * PI / 180.0f);
			fwd.z = std::sin(yaw * PI / 180.0f) * std::cos(pitch * PI / 180.0f);
			front = fwd.normalize();
			right = front.cross(worldUp).normalize();
			up = right.cross(front).normalize();
		}
	};
}