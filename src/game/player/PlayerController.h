#pragma once

#include "../../engine/Aabb.h"
#include "../../engine/vector.h"
#include "../collision/PlayerCollision.h"

#include <list>
#include <vector>

struct GLFWwindow;

class FirstPersonCamera;

// Liest Tastatur/Maus ein, bewegt den Spieler kollisionsgeprüft und erzeugt Frame-Aktionen
class PlayerController
{
public:
    PlayerController(GLFWwindow* window, FirstPersonCamera& camera);

    void update(float deltaTime);

    void setStaticColliders(const std::list<AABB>* colliders);

    void setDynamicColliders(const std::vector<AABB>* colliders);

    bool interactPressed() const;
    bool interactHeld() const;
    bool primaryActionPressed() const;
    bool specialActionPressed() const;

    bool isObjectRotationActive() const;

    void setObjectRotationAllowed(bool allowed);

    float mouseDeltaX() const;
    float mouseDeltaY() const;

    float playerRadius() const;

    void setGameplayInputEnabled(bool enabled);

    bool escapePressed() const;

    bool menuUpPressed() const;

    bool menuDownPressed() const;

    bool menuLeftPressed() const;

    bool menuRightPressed() const;

    bool confirmPressed() const;

private:
    void updateActionInput();

    void processKeyboardInput(float deltaTime);

    void processMouseInput();

    std::list<AABB> createMovementColliders(const Vector& currentPosition,
                                            const Vector& targetPosition) const;

    GLFWwindow* m_window;
    FirstPersonCamera& m_camera;

    PlayerCollision m_collision;

    const std::list<AABB>* m_staticColliders;

    const std::vector<AABB>* m_dynamicColliders;

    float m_movementSpeed;
    float m_playerRadius;
    float m_mouseSensitivity;

    float m_mouseDeltaX;
    float m_mouseDeltaY;

    double m_lastMouseX;
    double m_lastMouseY;

    bool m_firstMouse;

    bool m_interactPressed;
    bool m_interactHeld;
    bool m_eWasPressedLastFrame;

    bool m_primaryActionPressed;
    bool m_leftMouseWasPressedLastFrame;

    bool m_specialActionPressed;
    bool m_fWasPressedLastFrame;

    bool m_objectRotationActive;
    bool m_objectRotationAllowed;

    bool m_gameplayInputEnabled;

    bool m_escapePressed;
    bool m_escapeWasPressedLastFrame;

    bool m_menuUpPressed;
    bool m_menuUpWasPressedLastFrame;

    bool m_menuDownPressed;
    bool m_menuDownWasPressedLastFrame;

    bool m_menuLeftPressed;
    bool m_menuLeftWasPressedLastFrame;

    bool m_menuRightPressed;
    bool m_menuRightWasPressedLastFrame;

    bool m_confirmPressed;
    bool m_confirmWasPressedLastFrame;
};
