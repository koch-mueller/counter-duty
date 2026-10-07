#pragma once

#include "IPhysicsWorld.h"

#include <reactphysics3d/reactphysics3d.h>

#include <unordered_map>

// Implementiert IPhysicsWorld mit ReactPhysics3D und übernimmt die Typ-/Transform-Konvertierung
class ReactPhysicsWorld : public IPhysicsWorld
{
public:
    ReactPhysicsWorld();
    ~ReactPhysicsWorld() override;

    PhysicsBodyId createBox(const Matrix& transform,
                            const Vector& size,
                            PhysicsBodyType bodyType,
                            float mass) override;

    void removeBody(PhysicsBodyId bodyId) override;

    void setBodyType(PhysicsBodyId bodyId, PhysicsBodyType bodyType) override;

    void setBodyTransform(PhysicsBodyId bodyId, const Matrix& transform) override;

    Matrix getBodyTransform(PhysicsBodyId bodyId) const override;

    void stopBody(PhysicsBodyId bodyId) override;

    void wakeBody(PhysicsBodyId bodyId) override;

    bool isBodySleeping(PhysicsBodyId bodyId) const override;

    void update(float deltaTime) override;

private:
    // Zuordnung zwischen eigener Body-ID und den Objekten der ReactPhysics3D-Welt
    struct BodyData
    {
        reactphysics3d::RigidBody* body = nullptr;

        reactphysics3d::BoxShape* shape = nullptr;
    };

    static reactphysics3d::Vector3 toPhysicsVector(const Vector& vector);

    static reactphysics3d::Transform toPhysicsTransform(const Matrix& transform);

    static Matrix toGameTransform(const reactphysics3d::Transform& transform);

    static reactphysics3d::BodyType toPhysicsBodyType(PhysicsBodyType bodyType);

    BodyData* findBody(PhysicsBodyId bodyId);

    const BodyData* findBody(PhysicsBodyId bodyId) const;

    void destroyBodyData(BodyData& bodyData);

    reactphysics3d::PhysicsCommon m_physicsCommon;

    reactphysics3d::PhysicsWorld* m_world;

    std::unordered_map<PhysicsBodyId, BodyData> m_bodies;

    PhysicsBodyId m_nextBodyId;
};
