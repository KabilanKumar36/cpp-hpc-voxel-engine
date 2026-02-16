#pragma once
#include <cmath>
#include "MathUtils.h"
#include "Matrix.h"

namespace Core {

// Default Camera Values
constexpr float YAW = -90.0f;
constexpr float PITCH = 0.0f;
constexpr float SPEED = 2.5f;
constexpr float SENSITIVITY = 0.1f;
constexpr float ZOOM = 45.0f;
constexpr float PI = 3.1415926535f;

/**
 * @class Camera
 * @brief Processes input and calculates the View Matrix for OpenGL rendering.
 * * Implements a standard Euler Angle (Yaw/Pitch) FPS camera.
 */
class Camera {
public:
    /**
     * @brief Constructs the camera at a specific position.
     * @param startPos Initial world space position.
     * @param startUP World's "Up" direction (usually 0,1,0).
     */
    Camera(Vec3 startPos = Vec3(0.0f, 0.0f, 0.0f), Vec3 startUP = Vec3(0.0f, 1.0f, 0.0f))
        : m_objPtPosition(startPos), m_objVecFront(Vec3(0.0f, 0.0f, -1.0f)), m_objWorldUp(startUP) {
        UpdateCameraVectors();
    }

    /**
     * @brief Calculates the LookAt View Matrix.
     */
    Mat4 GetViewMatrix() {
        return Mat4::LookAt(m_objPtPosition, m_objPtPosition + m_objVecFront, m_objVecUp);
    }

    // --- Input Processing ---

    /**
     * @brief Moves the camera based on keyboard input direction.
     * @param iDirection Enum/Int representing direction (0=Fwd, 1=Back, 2=Left, 3=Right, 4=Up,
     * 5=Down).
     * @param fDeltaTime Time elapsed per frame.
     */
    void ProcessKeyboard(int iDirection, float fDeltaTime) {
        float velocity = m_fSpeed * fDeltaTime;
        if (iDirection == 0)
            m_objPtPosition += m_objVecFront * velocity;  // FORWARD
        if (iDirection == 1)
            m_objPtPosition -= m_objVecFront * velocity;  // BACKWARD
        if (iDirection == 2)
            m_objPtPosition -= m_objVecRight * velocity;  // LEFT
        if (iDirection == 3)
            m_objPtPosition += m_objVecRight * velocity;  // RIGHT
        if (iDirection == 4)
            m_objPtPosition += m_objVecUp * velocity;  // UP (Fly)
        if (iDirection == 5)
            m_objPtPosition -= m_objVecUp * velocity;  // DOWN (Fly)
    }

    /**
     * @brief Rotates the camera based on mouse movement.
     * @param fXOffset Mouse X delta.
     * @param fYOffset Mouse Y delta.
     * @param bConstrainPitch If true, prevents camera from flipping upside down.
     */
    void ProcessMouseMovement(float fXOffset, float fYOffset, bool bConstrainPitch = true) {
        fXOffset *= m_fSensitivity;
        fYOffset *= m_fSensitivity;

        m_fYaw += fXOffset;
        m_fPitch += fYOffset;

        if (bConstrainPitch) {
            if (m_fPitch > 89.0f)
                m_fPitch = 89.0f;
            if (m_fPitch < -89.0f)
                m_fPitch = -89.0f;
        }

        UpdateCameraVectors();
    }

    /**
     * @brief Pans the camera (Editor style movement).
     */
    void ProcessMousePan(float fXOffset, float fYOffset) {
        constexpr float fPanSpeed = 0.02f;
        m_objPtPosition -= m_objVecRight * (fXOffset * fPanSpeed);
        m_objPtPosition -= m_objVecUp * (fYOffset * fPanSpeed);
    }

    /**
     * @brief Zooms (changes FOV) based on scroll wheel.
     */
    void ProcessMouseScroll(float fYOffset) {
        m_fZoom -= fYOffset;
        if (m_fZoom < 1.0f)
            m_fZoom = 1.0f;
        if (m_fZoom > 90.0f)
            m_fZoom = 90.0f;
    }

    // --- Getters & Setters ---

    Vec3 GetUp() const { return m_objVecUp; }
    Vec3 GetFront() const { return m_objVecFront; }
    Vec3 GetRight() const { return m_objVecRight; }
    Vec3 GetCameraPosition() const { return m_objPtPosition; }
    float GetZoom() const { return m_fZoom; }

    void SetCameraPosition(const Vec3& position) { m_objPtPosition = position; }
    void SetCameraFront(const Vec3& front) { m_objVecFront = front.normalize(); }
    void SetCameraZoom(float zoom) { m_fZoom = zoom; }

    void SetCameraYawPitch(float yaw, float pitch) {
        m_fYaw = yaw;
        m_fPitch = pitch;
        UpdateCameraVectors();
    }

    /**
     * @brief Recalculates Front, Right, and Up vectors from Euler angles.
     */
    void UpdateCameraVectors() {
        Vec3 fwd;
        fwd.x = std::cos(m_fYaw * PI / 180.0f) * std::cos(m_fPitch * PI / 180.0f);
        fwd.y = std::sin(m_fPitch * PI / 180.0f);
        fwd.z = std::sin(m_fYaw * PI / 180.0f) * std::cos(m_fPitch * PI / 180.0f);
        m_objVecFront = fwd.normalize();
        // Gram-Schmidt process
        m_objVecRight = m_objVecFront.cross(m_objWorldUp).normalize();
        m_objVecUp = m_objVecRight.cross(m_objVecFront).normalize();
    }

private:
    // Camera Attributes
    Vec3 m_objPtPosition;
    Vec3 m_objVecFront;
    Vec3 m_objVecUp;
    Vec3 m_objVecRight;
    Vec3 m_objWorldUp;

    // Euler Angles
    float m_fYaw = YAW;
    float m_fPitch = PITCH;

    // Options
    float m_fZoom = ZOOM;

    float m_fSpeed = SPEED;
    float m_fSensitivity = SENSITIVITY;
};
}  // namespace Core