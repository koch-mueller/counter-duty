
#include "CarrySystem.h"

#include "../player/FirstPersonCamera.h"

namespace
{
    constexpr float kCarrySweepStep = 0.03f;

    constexpr float kDefaultHoldDistance = 1.2f;

    constexpr float kDefaultMinimumHoldDistance = 0.1f;

    constexpr float kHoldDistanceRecoverySpeed = 2.0f;

    constexpr float kRotationSpeed = 0.005f;

    constexpr float kDistanceSearchStep = 0.02f;

    bool overlapsPlayerXZ(const AABB& bounds, const Vector& playerPosition, float playerRadius)
    {
        float closestPointX = std::max(bounds.Min.X, std::min(playerPosition.X, bounds.Max.X));

        float closestPointZ = std::max(bounds.Min.Z, std::min(playerPosition.Z, bounds.Max.Z));

        float distanceX = playerPosition.X - closestPointX;

        float distanceZ = playerPosition.Z - closestPointZ;

        float distanceSquared = distanceX * distanceX + distanceZ * distanceZ;

        return distanceSquared <= playerRadius * playerRadius;
    }
}

CarrySystem::CarrySystem()
    : m_heldObject(nullptr),
      m_defaultHoldDistance(kDefaultHoldDistance),
      m_currentHoldDistance(kDefaultHoldDistance),
      m_minimumHoldDistance(kDefaultMinimumHoldDistance),
      m_holdDistanceRecoverySpeed(kHoldDistanceRecoverySpeed),
      m_horizontalOffset(0.0f),
      m_verticalOffset(0.0f),
      m_objectPitch(0.0f),
      m_objectYaw(0.0f),
      m_objectRoll(0.0f),
      m_rotationSpeed(kRotationSpeed),
      m_blockingColliders(nullptr),
      m_dynamicBlockingColliders(nullptr),
      m_hasLastValidTransform(false)
{
}

bool CarrySystem::pickUpObject(ICarryable* object)
{
    if (m_heldObject != nullptr || object == nullptr || !object->canBeCarried())
    {
        return false;
    }

    m_heldObject = object;

    m_objectPitch = 0.0f;
    m_objectYaw = 0.0f;
    m_objectRoll = 0.0f;

    m_hasLastValidTransform = false;

    m_heldObject->onCarryStarted();

    m_defaultHoldDistance = object->carryDistance();

    m_currentHoldDistance = m_defaultHoldDistance;

    m_minimumHoldDistance = object->minimumCarryDistance();

    m_verticalOffset = object->carryVerticalOffset();

    return true;
}

void CarrySystem::update(const FirstPersonCamera& camera, float deltaTime)
{
    if (m_heldObject == nullptr)
    {
        return;
    }

    float largestValidDistance = 0.0f;

    // Falls vor der Kamera keine gueltige Position existiert, bleibt das Objekt am letzten sicheren Transform
    if (!findLargestValidHoldDistance(camera, largestValidDistance))
    {
        if (m_hasLastValidTransform)
        {
            m_heldObject->setWorldTransform(m_lastValidTransform);
        }

        return;
    }

    // Vor Hindernissen wird das Objekt zur Kamera gezogen
    // Sobald wieder Platz ist, kehrt es langsam nach vorne zurück
    if (largestValidDistance < m_currentHoldDistance)
    {
        m_currentHoldDistance = largestValidDistance;
    }
    else if (largestValidDistance > m_currentHoldDistance)
    {
        m_currentHoldDistance += m_holdDistanceRecoverySpeed * deltaTime;

        if (m_currentHoldDistance > largestValidDistance)
        {
            m_currentHoldDistance = largestValidDistance;
        }
    }

    Matrix targetTransform = calculateTargetTransform(camera, m_currentHoldDistance);

    if (isTransformValid(*m_heldObject, targetTransform))
    {
        m_heldObject->setWorldTransform(targetTransform);

        m_lastValidTransform = targetTransform;

        m_hasLastValidTransform = true;

        return;
    }

    if (m_hasLastValidTransform)
    {
        m_heldObject->setWorldTransform(m_lastValidTransform);
    }
}

bool CarrySystem::isCarrying() const
{
    return m_heldObject != nullptr;
}

ICarryable* CarrySystem::heldObject() const
{
    return m_heldObject;
}

bool CarrySystem::canRotateHeldObject() const
{
    return m_heldObject != nullptr && m_heldObject->allowsCarryRotation();
}

void CarrySystem::setBlockingColliders(const std::list<AABB>& colliders)
{
    m_blockingColliders = &colliders;
}

void CarrySystem::setDynamicBlockingColliders(const std::vector<AABB>& colliders)
{
    m_dynamicBlockingColliders = &colliders;
}

void CarrySystem::rotateHeldObject(float mouseDeltaX, float mouseDeltaY)
{
    if (m_heldObject == nullptr || !m_heldObject->allowsCarryRotation())
    {
        return;
    }

    m_objectYaw += mouseDeltaX * m_rotationSpeed;

    m_objectPitch += mouseDeltaY * m_rotationSpeed;
}

bool CarrySystem::tryDropHeldObject(const Vector& playerPosition, float playerRadius)
{
    if (m_heldObject == nullptr)
    {
        return false;
    }

    Matrix currentTransform = m_heldObject->getWorldTransform();

    if (!isTransformValid(*m_heldObject, currentTransform))
    {
        return false;
    }

    // Grosse Objekte wie der Lieferkarton duerfen nicht so abgelegt werden, dass sie den Spieler einschliessen
    if (m_heldObject->requiresPlayerClearanceOnDrop())
    {
        AABB worldBounds = m_heldObject->getLocalBounds().transform(currentTransform);

        if (overlapsPlayerXZ(worldBounds, playerPosition, playerRadius))
        {
            return false;
        }
    }

    ICarryable* droppedObject = m_heldObject;

    m_heldObject = nullptr;

    resetCarryParameters();

    droppedObject->onCarryEnded();

    return true;
}

Matrix CarrySystem::calculateCameraRotation(const FirstPersonCamera& camera) const
{
    Vector right = camera.rightVector();

    Vector up = camera.up();

    Vector front = camera.frontVector();

    Matrix rotation;

    rotation.identity();

    // Die Kamerabasis bildet direkt die Orientierung des getragenen Objekts
    rotation.m00 = right.X;
    rotation.m10 = right.Y;
    rotation.m20 = right.Z;

    rotation.m01 = up.X;
    rotation.m11 = up.Y;
    rotation.m21 = up.Z;

    rotation.m02 = -front.X;
    rotation.m12 = -front.Y;
    rotation.m22 = -front.Z;

    return rotation;
}

Matrix CarrySystem::calculateTargetTransform(const FirstPersonCamera& camera,
                                             float holdDistance) const
{
    Vector carryFront = camera.frontVector();

    Vector carryUp = camera.up();

    // Lange Objekte koennen die Kameraneigung fuer ihre Position ignorieren, damit sie nicht unnatuerlich hochspringen
    if (m_heldObject != nullptr && !m_heldObject->usesCameraPitchForCarryPosition())
    {
        carryFront.Y = 0.0f;

        if (carryFront.lengthSquared() > 0.0f)
        {
            carryFront.normalize();
        }

        carryUp = Vector(0.0f, 1.0f, 0.0f);
    }

    Vector holdPosition = camera.position() + carryFront * holdDistance;

    holdPosition += camera.rightVector() * m_horizontalOffset;

    holdPosition += carryUp * m_verticalOffset;

    Matrix targetTransform;

    targetTransform.translation(holdPosition);

    if (m_heldObject == nullptr || m_heldObject->usesCameraRotation())
    {
        targetTransform *= calculateCameraRotation(camera);
    }

    targetTransform *= calculateObjectRotation();

    return targetTransform;
}

Matrix CarrySystem::calculateObjectRotation() const
{
    Matrix rotation;

    rotation.identity();

    rotation.rotationYawPitchRoll(m_objectYaw, m_objectPitch, m_objectRoll);

    return rotation;
}

bool CarrySystem::isTransformValid(const ICarryable& object, const Matrix& transform) const
{
    AABB worldBounds = object.getLocalBounds().transform(transform);

    if (m_blockingColliders != nullptr)
    {
        for (const AABB& collider : *m_blockingColliders)
        {
            if (worldBounds.overlaps(collider))
            {
                return false;
            }
        }
    }

    // Bewegliche Blocker werden nur fuer Objekte geprueft, die diese zusaetzliche Kollision benoetigen
    if (m_dynamicBlockingColliders != nullptr && object.usesDynamicBlockingColliders())
    {
        for (const AABB& collider : *m_dynamicBlockingColliders)
        {
            if (worldBounds.overlaps(collider))
            {
                return false;
            }
        }
    }

    return true;
}

bool CarrySystem::findLargestValidHoldDistance(const FirstPersonCamera& camera,
                                               float& validDistance) const
{
    if (m_heldObject == nullptr)
    {
        return false;
    }

    bool foundValidDistance = false;

    bool reachedObstacle = false;

    float largestValidDistance = m_minimumHoldDistance;

    // Die Suche läuft von der Kamera nach außen
    // Beim ersten Hindernis wird nicht dahinter weitergesucht
    for (float distance = m_minimumHoldDistance; distance <= m_defaultHoldDistance;
         distance += kDistanceSearchStep)
    {
        Matrix targetTransform = calculateTargetTransform(camera, distance);

        if (!isTransformValid(*m_heldObject, targetTransform))
        {
            reachedObstacle = true;

            break;
        }

        largestValidDistance = distance;

        foundValidDistance = true;
    }

    if (!foundValidDistance)
    {
        return false;
    }

    // Die Schrittweite trifft die Standarddistanz nicht immer exakt
    // Ohne Hindernis prüfen wir sie direkt
    if (!reachedObstacle)
    {
        Matrix defaultTransform = calculateTargetTransform(camera, m_defaultHoldDistance);

        if (isTransformValid(*m_heldObject, defaultTransform))
        {
            largestValidDistance = m_defaultHoldDistance;
        }
    }

    validDistance = largestValidDistance;

    return true;
}

void CarrySystem::resetCarryParameters()
{
    m_defaultHoldDistance = kDefaultHoldDistance;

    m_currentHoldDistance = kDefaultHoldDistance;

    m_minimumHoldDistance = kDefaultMinimumHoldDistance;

    m_verticalOffset = 0.0f;

    m_objectPitch = 0.0f;
    m_objectYaw = 0.0f;
    m_objectRoll = 0.0f;

    m_hasLastValidTransform = false;
}
