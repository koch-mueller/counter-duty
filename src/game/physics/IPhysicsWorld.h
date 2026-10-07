#pragma once

#include "../../engine/Matrix.h"
#include "../../engine/vector.h"

using PhysicsBodyId = unsigned int;

// Physiktyp unabhängig von der konkret verwendeten Physikbibliothek
enum class PhysicsBodyType
{
    Static,
    Kinematic,
    Dynamic
};

// Abstraktion der benötigten Physikfunktionen, damit Gameplay-Code nicht direkt von ReactPhysics3D abhängt
class IPhysicsWorld
{
public:
    virtual ~IPhysicsWorld() = default;

    virtual PhysicsBodyId createBox(const Matrix& transform,
                                    const Vector& size,
                                    PhysicsBodyType bodyType,
                                    float mass) = 0;

    virtual void removeBody(PhysicsBodyId bodyId) = 0;

    virtual void setBodyType(PhysicsBodyId bodyId, PhysicsBodyType bodyType) = 0;

    virtual void setBodyTransform(PhysicsBodyId bodyId, const Matrix& transform) = 0;

    virtual Matrix getBodyTransform(PhysicsBodyId bodyId) const = 0;

    virtual void stopBody(PhysicsBodyId bodyId) = 0;

    virtual void wakeBody(PhysicsBodyId bodyId) = 0;

    virtual bool isBodySleeping(PhysicsBodyId bodyId) const = 0;

    virtual void update(float deltaTime) = 0;
};
