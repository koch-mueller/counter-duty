#pragma once

#include "../../engine/Camera.h"

// First-Person-Kamera mit Blickrichtung und perspektivischer Projektion für das Gameplay
class FirstPersonCamera : public Camera
{
public:
    explicit FirstPersonCamera(GLFWwindow* window);

    void update() override;

    Vector frontVector() const;
    Vector rightVector() const;

    void rotate(float yawOffset, float pitchOffset);

private:
    Vector calculateFrontVector() const;
    Vector calculateRightVector() const;

    float m_pitch;
    float m_yaw;
};
