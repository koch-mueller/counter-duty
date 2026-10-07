#include "FirstPersonCamera.h"

#include <cmath>

namespace
{
    constexpr float kPi = 3.14159265358979323846f;

    constexpr float kMaximumPitch = kPi * 89.0f / 180.0f;
}

FirstPersonCamera::FirstPersonCamera(GLFWwindow* window)
    : Camera(window),
      m_pitch(0.0f),
      m_yaw(kPi * 0.5f)
{
    m_Position = Vector(0.0f, 1.7f, -9.5f);

    m_Target = Vector(0.0f, 1.7f, -8.5f);

    m_Up = Vector(0.0f, 1.0f, 0.0f);

    update();
}

void FirstPersonCamera::update()
{
    updateProjectionMatrix();

    Vector front = calculateFrontVector();

    Vector right = calculateRightVector();

    Vector up = right.cross(front);

    up.normalize();

    m_Target = m_Position + front;

    m_Up = up;

    m_ViewMatrix.lookAt(m_Target, m_Up, m_Position);
}

Vector FirstPersonCamera::frontVector() const
{
    return calculateFrontVector();
}

Vector FirstPersonCamera::rightVector() const
{
    return calculateRightVector();
}

void FirstPersonCamera::rotate(float yawOffset, float pitchOffset)
{
    m_yaw += yawOffset;

    m_pitch += pitchOffset;

    // Knapp unter 90 Grad begrenzen, damit Front- und Right-Vektor nicht ausartet
    if (m_pitch > kMaximumPitch)
    {
        m_pitch = kMaximumPitch;
    }

    if (m_pitch < -kMaximumPitch)
    {
        m_pitch = -kMaximumPitch;
    }
}

// Aus Yaw und Pitch wird ein normierter Blickrichtungsvektor im Weltkoordinatensystem berechnet
Vector FirstPersonCamera::calculateFrontVector() const
{
    Vector front;

    front.X = std::cos(m_yaw) * std::cos(m_pitch);

    front.Y = std::sin(m_pitch);

    front.Z = std::sin(m_yaw) * std::cos(m_pitch);

    front.normalize();

    return front;
}

Vector FirstPersonCamera::calculateRightVector() const
{
    Vector front = calculateFrontVector();

    const Vector worldUp(0.0f, 1.0f, 0.0f);

    Vector right = front.cross(worldUp);

    right.normalize();

    return right;
}
