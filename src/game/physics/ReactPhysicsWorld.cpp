
#include "ReactPhysicsWorld.h"

namespace
{
    constexpr float kGravity = -9.81f;

    constexpr float kBounciness = 0.05f;

    constexpr float kFriction = 0.7f;
}

ReactPhysicsWorld::ReactPhysicsWorld()
    : m_world(nullptr),
      m_nextBodyId(1)
{
    m_world = m_physicsCommon.createPhysicsWorld();

    if (m_world == nullptr)
    {
        return;
    }

    m_world->setGravity(reactphysics3d::Vector3(0.0f, kGravity, 0.0f));
}

ReactPhysicsWorld::~ReactPhysicsWorld()
{
    for (auto& bodyEntry : m_bodies)
    {
        destroyBodyData(bodyEntry.second);
    }

    m_bodies.clear();

    if (m_world == nullptr)
    {
        return;
    }

    m_physicsCommon.destroyPhysicsWorld(m_world);

    m_world = nullptr;
}

PhysicsBodyId ReactPhysicsWorld::createBox(const Matrix& transform,
                                           const Vector& size,
                                           PhysicsBodyType bodyType,
                                           float mass)
{
    if (m_world == nullptr)
    {
        return 0;
    }

    bool hasValidSize = size.X > 0.0f && size.Y > 0.0f && size.Z > 0.0f;

    if (!hasValidSize)
    {
        return 0;
    }

    if (bodyType == PhysicsBodyType::Dynamic && mass <= 0.0f)
    {
        return 0;
    }

    reactphysics3d::Transform physicsTransform = toPhysicsTransform(transform);

    // RigidBody und BoxShape werden getrennt gespeichert, weil beide über PhysicsCommon explizit freigegeben werden
    reactphysics3d::RigidBody* rigidBody = m_world->createRigidBody(physicsTransform);

    if (rigidBody == nullptr)
    {
        return 0;
    }

    Vector halfSize = size * 0.5f;

    reactphysics3d::BoxShape* boxShape = m_physicsCommon.createBoxShape(toPhysicsVector(halfSize));

    if (boxShape == nullptr)
    {
        m_world->destroyRigidBody(rigidBody);

        return 0;
    }

    reactphysics3d::Collider* collider =
        rigidBody->addCollider(boxShape, reactphysics3d::Transform ::identity());

    if (collider == nullptr)
    {
        m_world->destroyRigidBody(rigidBody);

        m_physicsCommon.destroyBoxShape(boxShape);

        return 0;
    }

    reactphysics3d::Material& material = collider->getMaterial();

    material.setBounciness(kBounciness);

    material.setFrictionCoefficient(kFriction);

    PhysicsBodyId bodyId = m_nextBodyId;

    ++m_nextBodyId;

    BodyData bodyData;

    bodyData.body = rigidBody;

    bodyData.shape = boxShape;

    m_bodies[bodyId] = bodyData;

    rigidBody->setType(toPhysicsBodyType(bodyType));

    if (bodyType == PhysicsBodyType::Dynamic)
    {
        // ReactPhysics3D berechnet die Masse aus Dichte und Collider-Volumen; deshalb wird die gewünschte Masse umgerechnet
        float volume = size.X * size.Y * size.Z;

        float density = mass / volume;

        material.setMassDensity(density);

        rigidBody->updateMassPropertiesFromColliders();
    }

    return bodyId;
}

void ReactPhysicsWorld::removeBody(PhysicsBodyId bodyId)
{
    auto bodyIterator = m_bodies.find(bodyId);

    if (bodyIterator == m_bodies.end())
    {
        return;
    }

    destroyBodyData(bodyIterator->second);

    m_bodies.erase(bodyIterator);
}

void ReactPhysicsWorld::setBodyType(PhysicsBodyId bodyId, PhysicsBodyType bodyType)
{
    BodyData* bodyData = findBody(bodyId);

    if (bodyData == nullptr || bodyData->body == nullptr)
    {
        return;
    }

    bodyData->body->setType(toPhysicsBodyType(bodyType));
}

void ReactPhysicsWorld::setBodyTransform(PhysicsBodyId bodyId, const Matrix& transform)
{
    BodyData* bodyData = findBody(bodyId);

    if (bodyData == nullptr || bodyData->body == nullptr)
    {
        return;
    }

    bodyData->body->setTransform(toPhysicsTransform(transform));
}

Matrix ReactPhysicsWorld::getBodyTransform(PhysicsBodyId bodyId) const
{
    const BodyData* bodyData = findBody(bodyId);

    if (bodyData == nullptr || bodyData->body == nullptr)
    {
        Matrix identityTransform;

        identityTransform.identity();

        return identityTransform;
    }

    return toGameTransform(bodyData->body->getTransform());
}

void ReactPhysicsWorld::stopBody(PhysicsBodyId bodyId)
{
    BodyData* bodyData = findBody(bodyId);

    if (bodyData == nullptr || bodyData->body == nullptr)
    {
        return;
    }

    const reactphysics3d::Vector3 zeroVelocity(0.0f, 0.0f, 0.0f);

    bodyData->body->setLinearVelocity(zeroVelocity);

    bodyData->body->setAngularVelocity(zeroVelocity);

    bodyData->body->setIsSleeping(false);
}

void ReactPhysicsWorld::wakeBody(PhysicsBodyId bodyId)
{
    BodyData* bodyData = findBody(bodyId);

    if (bodyData == nullptr || bodyData->body == nullptr)
    {
        return;
    }

    bodyData->body->setIsSleeping(false);
}

bool ReactPhysicsWorld::isBodySleeping(PhysicsBodyId bodyId) const
{
    const BodyData* bodyData = findBody(bodyId);

    if (bodyData == nullptr || bodyData->body == nullptr)
    {
        return false;
    }

    return bodyData->body->isSleeping();
}

void ReactPhysicsWorld::update(float deltaTime)
{
    if (m_world == nullptr || deltaTime <= 0.0f)
    {
        return;
    }

    m_world->update(deltaTime);
}

reactphysics3d::Vector3 ReactPhysicsWorld::toPhysicsVector(const Vector& vector)
{
    return reactphysics3d::Vector3(vector.X, vector.Y, vector.Z);
}

reactphysics3d::Transform ReactPhysicsWorld::toPhysicsTransform(const Matrix& transform)
{
    // Nur Translation und Rotation werden übernommen; Skalierung steckt bereits in der Größe des BoxShape
    reactphysics3d::Vector3 position(transform.m03, transform.m13, transform.m23);

    reactphysics3d::Matrix3x3 rotation(transform.m00,
                                       transform.m01,
                                       transform.m02,
                                       transform.m10,
                                       transform.m11,
                                       transform.m12,
                                       transform.m20,
                                       transform.m21,
                                       transform.m22);

    return reactphysics3d::Transform(position, rotation);
}

Matrix ReactPhysicsWorld::toGameTransform(const reactphysics3d::Transform& transform)
{
    Matrix gameTransform;

    // ReactPhysics3D kann die Transform-Matrix direkt im von OpenGL erwarteten Layout ausgeben
    transform.getOpenGLMatrix(gameTransform.m);

    return gameTransform;
}

reactphysics3d::BodyType ReactPhysicsWorld::toPhysicsBodyType(PhysicsBodyType bodyType)
{
    switch (bodyType)
    {
        case PhysicsBodyType::Static:
            return reactphysics3d ::BodyType::STATIC;

        case PhysicsBodyType::Kinematic:
            return reactphysics3d ::BodyType::KINEMATIC;

        case PhysicsBodyType::Dynamic:
            return reactphysics3d ::BodyType::DYNAMIC;
    }

    return reactphysics3d ::BodyType::STATIC;
}

ReactPhysicsWorld::BodyData* ReactPhysicsWorld::findBody(PhysicsBodyId bodyId)
{
    auto bodyIterator = m_bodies.find(bodyId);

    if (bodyIterator == m_bodies.end())
    {
        return nullptr;
    }

    return &bodyIterator->second;
}

const ReactPhysicsWorld::BodyData* ReactPhysicsWorld::findBody(PhysicsBodyId bodyId) const
{
    auto bodyIterator = m_bodies.find(bodyId);

    if (bodyIterator == m_bodies.end())
    {
        return nullptr;
    }

    return &bodyIterator->second;
}

void ReactPhysicsWorld::destroyBodyData(BodyData& bodyData)
{
    // Erst den Body und danach die separat erzeugte Shape freigeben
    if (bodyData.body != nullptr)
    {
        if (m_world != nullptr)
        {
            m_world->destroyRigidBody(bodyData.body);
        }

        bodyData.body = nullptr;
    }

    if (bodyData.shape != nullptr)
    {
        m_physicsCommon.destroyBoxShape(bodyData.shape);

        bodyData.shape = nullptr;
    }
}
