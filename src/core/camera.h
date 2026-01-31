#pragma once
#include "MathUtils.h"
#include "Matrix.h"
#include <cmath>

namespace Core {
	constexpr float PI = 3.1415926535f;
	constexpr float YAW = -90.0f;
	constexpr float PITCH = 0.0f;
	constexpr float SPEED = 2.5f;
	constexpr float SENSITIVITY = 0.1f;
	constexpr float ZOOM = 45.0f;
	class Camera {
	public:
		//Constructor
		Camera(Vec3 startPos = Vec3(0.0f, 0.0f, 0.0f), Vec3 startUP = Vec3(0.0f, 1.0f, 0.0f))
			: m_objPtPosition(startPos), m_objVecFront(Vec3(0.0f, 0.0f, -1.0f)), m_objWorldUp(startUP) {
		
			UpdateCameraVectors();
		}

		Mat4 getViewMatrix() {
			return Mat4::lookAt(m_objPtPosition, m_objPtPosition + m_objVecFront, m_objVecUp);
		}

		void processKeyboard(int iDirection, float fdeltaTime)
		{
			float velocity = m_fSpeed * fdeltaTime;
			if (iDirection == 0) //FORWARD
				m_objPtPosition = m_objPtPosition + m_objVecFront * velocity;
			if (iDirection == 1) //BACKWARD
				m_objPtPosition = m_objPtPosition - m_objVecFront * velocity;
			if (iDirection == 2) //LEFT
				m_objPtPosition = m_objPtPosition - m_objVecRight * velocity;
			if (iDirection == 3) //RIGHT
				m_objPtPosition = m_objPtPosition + m_objVecRight * velocity;
			if (iDirection == 4) //UP
				m_objPtPosition = m_objPtPosition + m_objVecUp * velocity;
			if (iDirection == 5) //DOWN
				m_objPtPosition = m_objPtPosition - m_objVecUp * velocity;
		}

		void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true)
		{
			xOffset *= m_fSensitivity;
			yOffset *= m_fSensitivity;
			m_fYaw += xOffset;
			m_fPitch += yOffset;
			if (constrainPitch)
			{
				if (m_fPitch > 89.0f)
					m_fPitch = 89.0f;
				if (m_fPitch < -89.0f)
					m_fPitch = -89.0f;
			}
			UpdateCameraVectors();
		}
		void processMousePan(float fXOffset, float fYOffset) {
			float fPanSpeed = 0.02f;

			m_objPtPosition -= m_objVecRight * (fXOffset * fPanSpeed);
			m_objPtPosition -= m_objVecUp * (fYOffset * fPanSpeed);
#if DEBUG
			// DEBUG: Print values to console
			std::cout << "PanX: " << m_objPtPosition.x << " | Yaw: " << m_fYaw << std::endl;
			std::cout << "PanY: " << m_objPtPosition.y << " | Pitch: " << m_fPitch << std::endl;
#endif
		}
		void ProcessMouseScroll(float fYOffset) {
			m_fZoom -= fYOffset;
			if (m_fZoom < 1.0f)
				m_fZoom = 1.0f;
			if (m_fZoom > 90.0f)
				m_fZoom = 90.0f;
		}
		float GetZoom() const {
			return m_fZoom;
		}
		void UpdateCameraVectors() {
			Vec3 fwd;
			fwd.x = std::cos(m_fYaw * PI / 180.0f) * std::cos(m_fPitch * PI / 180.0f);
			fwd.y = std::sin(m_fPitch * PI / 180.0f);
			fwd.z = std::sin(m_fYaw * PI / 180.0f) * std::cos(m_fPitch * PI / 180.0f);
			m_objVecFront = fwd.normalize();
			m_objVecRight = m_objVecFront.cross(m_objWorldUp).normalize();
			m_objVecUp = m_objVecRight.cross(m_objVecFront).normalize();
		}
		void SetCameraPosition(const Vec3& position) {
			m_objPtPosition = position;
		}
		void SetCameraFront(const Vec3& front) {
			m_objVecFront = front.normalize();
		}
		void SetCameraYawPitch(float yaw, float pitch) {
			m_fYaw = yaw;
			m_fPitch = pitch;
		}
		void SetCameraZoom(float zoom) {
			m_fZoom = zoom;
		}
	private:
		Vec3 m_objPtPosition;
		Vec3 m_objVecFront;
		Vec3 m_objVecUp;
		Vec3 m_objVecRight;
		Vec3 m_objWorldUp;

		//Euler Angles
		float m_fYaw = YAW;
		float m_fPitch = PITCH;
		float m_fZoom = ZOOM;

		float m_fSpeed = SPEED;
		float m_fSensitivity = SENSITIVITY;

	};
}