#include "PlayerController.h"

#include "FirstPersonCamera.h"

#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>

#include <algorithm>

namespace
{
    float calculateOverlapDepthXZ(const Vector& position, float radius, const AABB& collider)
    {
        float minimumX = collider.Min.X - radius;

        float maximumX = collider.Max.X + radius;

        float minimumZ = collider.Min.Z - radius;

        float maximumZ = collider.Max.Z + radius;

        if (position.X <= minimumX || position.X >= maximumX || position.Z <= minimumZ
            || position.Z >= maximumZ)
        {
            return 0.0f;
        }

        float overlapDepth = position.X - minimumX;

        overlapDepth = std::min(overlapDepth, maximumX - position.X);

        overlapDepth = std::min(overlapDepth, position.Z - minimumZ);

        overlapDepth = std::min(overlapDepth, maximumZ - position.Z);

        return overlapDepth;
    }
}

PlayerController::PlayerController(GLFWwindow* window, FirstPersonCamera& camera)
    : m_window(window),
      m_camera(camera),
      m_staticColliders(nullptr),
      m_dynamicColliders(nullptr),
      m_movementSpeed(5.0f),
      m_playerRadius(0.35f),
      m_mouseSensitivity(0.002f),
      m_mouseDeltaX(0.0f),
      m_mouseDeltaY(0.0f),
      m_lastMouseX(0.0),
      m_lastMouseY(0.0),
      m_firstMouse(true),
      m_interactPressed(false),
      m_interactHeld(false),
      m_eWasPressedLastFrame(false),
      m_primaryActionPressed(false),
      m_leftMouseWasPressedLastFrame(false),
      m_specialActionPressed(false),
      m_fWasPressedLastFrame(false),
      m_objectRotationActive(false),
      m_objectRotationAllowed(true),
      m_gameplayInputEnabled(true),
      m_escapePressed(false),
      m_escapeWasPressedLastFrame(false),
      m_menuUpPressed(false),
      m_menuUpWasPressedLastFrame(false),
      m_menuDownPressed(false),
      m_menuDownWasPressedLastFrame(false),
      m_menuLeftPressed(false),
      m_menuLeftWasPressedLastFrame(false),
      m_menuRightPressed(false),
      m_menuRightWasPressedLastFrame(false),
      m_confirmPressed(false),
      m_confirmWasPressedLastFrame(false)
{
    if (m_window != nullptr)
    {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void PlayerController::update(float deltaTime)
{
    if (m_window == nullptr)
    {
        return;
    }

    const float maximumDeltaTime = 0.1f;

    if (deltaTime > maximumDeltaTime)
    {
        deltaTime = maximumDeltaTime;
    }

    updateActionInput();

    processKeyboardInput(deltaTime);

    processMouseInput();
}

void PlayerController::setStaticColliders(const std::list<AABB>* colliders)
{
    m_staticColliders = colliders;
}

void PlayerController::setDynamicColliders(const std::vector<AABB>* colliders)
{
    m_dynamicColliders = colliders;
}

bool PlayerController::interactPressed() const
{
    return m_interactPressed;
}

bool PlayerController::interactHeld() const
{
    return m_interactHeld;
}

bool PlayerController::primaryActionPressed() const
{
    return m_primaryActionPressed;
}

bool PlayerController::specialActionPressed() const
{
    return m_specialActionPressed;
}

bool PlayerController::isObjectRotationActive() const
{
    return m_objectRotationActive;
}

void PlayerController::setObjectRotationAllowed(bool allowed)
{
    m_objectRotationAllowed = allowed;
}

void PlayerController::setGameplayInputEnabled(bool enabled)
{
    m_gameplayInputEnabled = enabled;

    if (!m_gameplayInputEnabled)
    {
        m_objectRotationActive = false;
    }
}

bool PlayerController::escapePressed() const
{
    return m_escapePressed;
}

bool PlayerController::menuUpPressed() const
{
    return m_menuUpPressed;
}

bool PlayerController::menuDownPressed() const
{
    return m_menuDownPressed;
}

bool PlayerController::menuLeftPressed() const
{
    return m_menuLeftPressed;
}

bool PlayerController::menuRightPressed() const
{
    return m_menuRightPressed;
}

bool PlayerController::confirmPressed() const
{
    return m_confirmPressed;
}

float PlayerController::mouseDeltaX() const
{
    return m_mouseDeltaX;
}

float PlayerController::mouseDeltaY() const
{
    return m_mouseDeltaY;
}

float PlayerController::playerRadius() const
{
    return m_playerRadius;
}

void PlayerController::updateActionInput()
{
    bool eIsPressed = glfwGetKey(m_window, GLFW_KEY_E) == GLFW_PRESS;

    // Aktionen wie Aufnehmen, Scannen und Menübestätigung werden flankengesteuert und lösen pro Tastendruck nur einmal aus
    bool ePressedThisFrame = eIsPressed && !m_eWasPressedLastFrame;

    m_interactPressed = m_gameplayInputEnabled && ePressedThisFrame;

    m_interactHeld = m_gameplayInputEnabled && eIsPressed;

    m_eWasPressedLastFrame = eIsPressed;

    bool leftMouseIsPressed = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    m_primaryActionPressed =
        m_gameplayInputEnabled && leftMouseIsPressed && !m_leftMouseWasPressedLastFrame;

    m_leftMouseWasPressedLastFrame = leftMouseIsPressed;

    bool fIsPressed = glfwGetKey(m_window, GLFW_KEY_F) == GLFW_PRESS;

    m_specialActionPressed = m_gameplayInputEnabled && fIsPressed && !m_fWasPressedLastFrame;

    m_fWasPressedLastFrame = fIsPressed;

    bool rightMouseIsPressed = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    m_objectRotationActive =
        m_gameplayInputEnabled && m_objectRotationAllowed && rightMouseIsPressed;

    bool escapeIsPressed = glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS;

    m_escapePressed = escapeIsPressed && !m_escapeWasPressedLastFrame;

    m_escapeWasPressedLastFrame = escapeIsPressed;

    bool menuUpIsPressed = glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS
                           || glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS;

    m_menuUpPressed = menuUpIsPressed && !m_menuUpWasPressedLastFrame;

    m_menuUpWasPressedLastFrame = menuUpIsPressed;

    bool menuDownIsPressed = glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS
                             || glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS;

    m_menuDownPressed = menuDownIsPressed && !m_menuDownWasPressedLastFrame;

    m_menuDownWasPressedLastFrame = menuDownIsPressed;

    bool menuLeftIsPressed = glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS
                             || glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS;

    m_menuLeftPressed = menuLeftIsPressed && !m_menuLeftWasPressedLastFrame;

    m_menuLeftWasPressedLastFrame = menuLeftIsPressed;

    bool menuRightIsPressed = glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS
                              || glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS;

    m_menuRightPressed = menuRightIsPressed && !m_menuRightWasPressedLastFrame;

    m_menuRightWasPressedLastFrame = menuRightIsPressed;

    bool confirmIsPressed = eIsPressed || glfwGetKey(m_window, GLFW_KEY_ENTER) == GLFW_PRESS;

    m_confirmPressed = confirmIsPressed && !m_confirmWasPressedLastFrame;

    m_confirmWasPressedLastFrame = confirmIsPressed;
}

void PlayerController::processKeyboardInput(float deltaTime)
{
    if (!m_gameplayInputEnabled)
    {
        return;
    }

    float movementDistance = m_movementSpeed * deltaTime;

    Vector front = m_camera.frontVector();

    front.Y = 0.0f;
    front.normalize();

    Vector right = m_camera.rightVector();

    right.Y = 0.0f;
    right.normalize();

    Vector movement(0.0f, 0.0f, 0.0f);

    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
    {
        movement += front;
    }

    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
    {
        movement += -front;
    }

    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
    {
        movement += -right;
    }

    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
    {
        movement += right;
    }

    if (movement.lengthSquared() <= 0.000001f)
    {
        return;
    }

    // Durch das Normalisieren bleibt diagonale Bewegung genauso schnell wie gerade Bewegung
    movement.normalize();

    movement = movement * movementDistance;

    Vector currentPosition = m_camera.position();

    Vector targetPosition = currentPosition + movement;

    std::list<AABB> movementColliders = createMovementColliders(currentPosition, targetPosition);

    if (movementColliders.empty())
    {
        m_camera.setPosition(targetPosition);

        return;
    }

    Vector newPosition =
        m_collision.move(currentPosition, movement, m_playerRadius, movementColliders);

    m_camera.setPosition(newPosition);
}

void PlayerController::processMouseInput()
{
    m_mouseDeltaX = 0.0f;
    m_mouseDeltaY = 0.0f;

    double mouseX = 0.0;
    double mouseY = 0.0;

    glfwGetCursorPos(m_window, &mouseX, &mouseY);

    if (m_firstMouse)
    {
        m_lastMouseX = mouseX;
        m_lastMouseY = mouseY;

        m_firstMouse = false;

        return;
    }

    float offsetX = static_cast<float>(mouseX - m_lastMouseX);

    float offsetY = static_cast<float>(m_lastMouseY - mouseY);

    m_lastMouseX = mouseX;
    m_lastMouseY = mouseY;

    if (!m_gameplayInputEnabled)
    {
        return;
    }

    m_mouseDeltaX = offsetX;
    m_mouseDeltaY = offsetY;

    // Im Rotationsmodus gehen die Mausdeltas an das CarrySystem; die Kamera bleibt in diesem Frame stehen
    if (m_objectRotationActive)
    {
        return;
    }

    offsetX *= m_mouseSensitivity;

    offsetY *= m_mouseSensitivity;

    m_camera.rotate(offsetX, offsetY);
}

std::list<AABB> PlayerController::createMovementColliders(const Vector& currentPosition,
                                                          const Vector& targetPosition) const
{
    std::list<AABB> colliders;

    if (m_staticColliders != nullptr)
    {
        colliders.insert(colliders.end(), m_staticColliders->begin(), m_staticColliders->end());
    }

    if (m_dynamicColliders == nullptr)
    {
        return colliders;
    }

    const float overlapTolerance = 0.0001f;

    for (const AABB& collider : *m_dynamicColliders)
    {
        float currentOverlapDepth =
            calculateOverlapDepthXZ(currentPosition, m_playerRadius, collider);

        float targetOverlapDepth =
            calculateOverlapDepthXZ(targetPosition, m_playerRadius, collider);

        // Wenn sich ein beweglicher Collider um den Spieler gelegt hat, darf er ihn nicht festsetzen: Bewegung nach außen bleibt erlaubt
        bool isAlreadyOverlapping = currentOverlapDepth > overlapTolerance;

        bool movesOutOfCollider = targetOverlapDepth + overlapTolerance < currentOverlapDepth;

        // Ein dynamischer Collider darf den Spieler nicht in sich einschließen
        // Herauslaufen bleibt erlaubt
        if (isAlreadyOverlapping && movesOutOfCollider)
        {
            continue;
        }

        colliders.push_back(collider);
    }

    return colliders;
}
