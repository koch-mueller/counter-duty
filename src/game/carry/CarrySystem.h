#pragma once

#include "../../engine/Aabb.h"
#include "../../engine/Matrix.h"
#include "ICarryable.h"

#include <list>
#include <vector>

class FirstPersonCamera;

// Verwaltet Aufnehmen, Halten, Rotieren und Ablegen tragbarer Objekte
// Die Haltedistanz wird bei Hindernissen automatisch angepasst
class CarrySystem
{
public:
    CarrySystem();

    bool pickUpObject(ICarryable* object);

    void update(const FirstPersonCamera& camera, float deltaTime);

    bool isCarrying() const;

    ICarryable* heldObject() const;

    bool canRotateHeldObject() const;

    void setBlockingColliders(const std::list<AABB>& colliders);

    void setDynamicBlockingColliders(const std::vector<AABB>& colliders);

    void rotateHeldObject(float mouseDeltaX, float mouseDeltaY);

    bool tryDropHeldObject(const Vector& playerPosition, float playerRadius);

private:
    Matrix calculateCameraRotation(const FirstPersonCamera& camera) const;

    Matrix calculateTargetTransform(const FirstPersonCamera& camera, float holdDistance) const;

    Matrix calculateObjectRotation() const;

    bool isTransformValid(const ICarryable& object, const Matrix& transform) const;

    bool findLargestValidHoldDistance(const FirstPersonCamera& camera, float& validDistance) const;

    void resetCarryParameters();

    ICarryable* m_heldObject;

    float m_defaultHoldDistance;
    float m_currentHoldDistance;
    float m_minimumHoldDistance;
    float m_holdDistanceRecoverySpeed;

    float m_horizontalOffset;
    float m_verticalOffset;

    float m_objectPitch;
    float m_objectYaw;
    float m_objectRoll;
    float m_rotationSpeed;

    const std::list<AABB>* m_blockingColliders;

    const std::vector<AABB>* m_dynamicBlockingColliders;

    Matrix m_lastValidTransform;

    bool m_hasLastValidTransform;
};
