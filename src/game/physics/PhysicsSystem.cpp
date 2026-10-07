
#include "PhysicsSystem.h"

#include "ReactPhysicsWorld.h"
#include "../products/Product.h"

namespace
{
    constexpr float kFixedTimeStep = 1.0f / 60.0f;

    constexpr float kMaximumDeltaTime = 0.1f;

    constexpr float kProductMass = 1.0f;
}

PhysicsSystem::PhysicsSystem()
    : m_world(std::make_unique<ReactPhysicsWorld>()),
      m_timeAccumulator(0.0f),
      m_fixedTimeStep(kFixedTimeStep)
{
}

PhysicsSystem::~PhysicsSystem() = default;

void PhysicsSystem::update(float deltaTime)
{
    if (m_world == nullptr)
    {
        return;
    }

    if (deltaTime < 0.0f)
    {
        deltaTime = 0.0f;
    }

    // Sehr lange Frames werden begrenzt, damit die Physiksimulation nach Hängern nicht instabil nachholt
    if (deltaTime > kMaximumDeltaTime)
    {
        deltaTime = kMaximumDeltaTime;
    }

    m_timeAccumulator += deltaTime;

    // Ein fester 60-Hz-Schritt macht die Simulation unabhängig von der Render-Framerate
    while (m_timeAccumulator >= m_fixedTimeStep)
    {
        m_world->update(m_fixedTimeStep);

        m_timeAccumulator -= m_fixedTimeStep;
    }

    synchronizeProductTransforms();
}

void PhysicsSystem::updateCarriedProduct(Product* product)
{
    if (product == nullptr || m_world == nullptr)
    {
        return;
    }

    PhysicsBodyId bodyId = findProductBody(product);

    if (bodyId == 0)
    {
        return;
    }

    m_world->setBodyTransform(bodyId, product->getWorldTransform());
}

PhysicsBodyId PhysicsSystem::addStaticBox(const Vector& center, const Vector& size)
{
    if (m_world == nullptr)
    {
        return 0;
    }

    Matrix transform;

    transform.translation(center);

    return m_world->createBox(transform, size, PhysicsBodyType::Static, 0.0f);
}

PhysicsBodyId PhysicsSystem::addKinematicBox(const Matrix& transform, const Vector& size)
{
    if (m_world == nullptr)
    {
        return 0;
    }

    return m_world->createBox(transform, size, PhysicsBodyType::Kinematic, 0.0f);
}

PhysicsBodyId PhysicsSystem::addDynamicBox(const Matrix& transform, const Vector& size, float mass)
{
    if (m_world == nullptr || mass <= 0.0f)
    {
        return 0;
    }

    return m_world->createBox(transform, size, PhysicsBodyType::Dynamic, mass);
}

void PhysicsSystem::startCarrying(Product* product)
{
    if (product == nullptr || m_world == nullptr)
    {
        return;
    }

    PhysicsBodyId bodyId = findProductBody(product);

    if (bodyId == 0)
    {
        return;
    }

    // Getragene Produkte werden kinematisch: ihr Transform kommt vom CarrySystem, nicht von der Schwerkraft
    m_world->setBodyType(bodyId, PhysicsBodyType::Kinematic);

    m_world->stopBody(bodyId);

    m_world->setBodyTransform(bodyId, product->getWorldTransform());
}

void PhysicsSystem::releaseProduct(Product* product)
{
    if (product == nullptr || m_world == nullptr)
    {
        return;
    }

    PhysicsBodyId bodyId = findProductBody(product);

    if (bodyId == 0)
    {
        bodyId = createProductBody(*product);

        if (bodyId == 0)
        {
            return;
        }

        m_productBodies[product] = bodyId;

        return;
    }

    // Beim Loslassen startet die Dynamik exakt an der zuletzt sichtbaren Carry-Position und ohne alte Geschwindigkeit
    m_world->setBodyTransform(bodyId, product->getWorldTransform());

    m_world->stopBody(bodyId);

    m_world->setBodyType(bodyId, PhysicsBodyType::Dynamic);

    m_world->wakeBody(bodyId);
}

void PhysicsSystem::removeProduct(Product* product)
{
    if (product == nullptr || m_world == nullptr)
    {
        return;
    }

    auto bodyIterator = m_productBodies.find(product);

    if (bodyIterator == m_productBodies.end())
    {
        return;
    }

    m_world->removeBody(bodyIterator->second);

    m_productBodies.erase(bodyIterator);
}

void PhysicsSystem::startCarryingBody(PhysicsBodyId bodyId, const Matrix& transform)
{
    if (m_world == nullptr || bodyId == 0)
    {
        return;
    }

    m_world->setBodyType(bodyId, PhysicsBodyType::Kinematic);

    m_world->stopBody(bodyId);

    m_world->setBodyTransform(bodyId, transform);
}

void PhysicsSystem::releaseBody(PhysicsBodyId bodyId, const Matrix& transform)
{
    if (m_world == nullptr || bodyId == 0)
    {
        return;
    }

    m_world->setBodyTransform(bodyId, transform);

    m_world->stopBody(bodyId);

    m_world->setBodyType(bodyId, PhysicsBodyType::Dynamic);

    m_world->wakeBody(bodyId);
}

Matrix PhysicsSystem::getBodyTransform(PhysicsBodyId bodyId) const
{
    if (m_world == nullptr || bodyId == 0)
    {
        Matrix identityTransform;

        identityTransform.identity();

        return identityTransform;
    }

    return m_world->getBodyTransform(bodyId);
}

void PhysicsSystem::updateBodyTransform(PhysicsBodyId bodyId, const Matrix& transform)
{
    if (m_world == nullptr || bodyId == 0)
    {
        return;
    }

    m_world->setBodyTransform(bodyId, transform);
}

void PhysicsSystem::removeBody(PhysicsBodyId bodyId)
{
    if (m_world == nullptr || bodyId == 0)
    {
        return;
    }

    m_world->removeBody(bodyId);
}

PhysicsBodyId PhysicsSystem::findProductBody(Product* product) const
{
    if (product == nullptr)
    {
        return 0;
    }

    auto bodyIterator = m_productBodies.find(product);

    if (bodyIterator == m_productBodies.end())
    {
        return 0;
    }

    return bodyIterator->second;
}

PhysicsBodyId PhysicsSystem::createProductBody(Product& product)
{
    if (m_world == nullptr)
    {
        return 0;
    }

    Vector size = product.getLocalBounds().size();

    return m_world->createBox(product.getWorldTransform(),
                              size,
                              PhysicsBodyType::Dynamic,
                              kProductMass);
}

void PhysicsSystem::synchronizeProductTransforms()
{
    if (m_world == nullptr)
    {
        return;
    }

    for (const auto& productBody : m_productBodies)
    {
        Product* product = productBody.first;

        PhysicsBodyId bodyId = productBody.second;

        if (product == nullptr || bodyId == 0)
        {
            continue;
        }

        // Nach jedem Physikschritt ist die Physikwelt die Quelle für Position und Rotation dynamischer Produkte
        Matrix transform = m_world->getBodyTransform(bodyId);

        product->setWorldTransform(transform);

        if (product->state() == ProductState::Held)
        {
            continue;
        }

        // Der Sleeping-Zustand wird als Produktstatus genutzt, um ruhende Objekte nicht unnötig weiterzubehandeln
        bool isSleeping = m_world->isBodySleeping(bodyId);

        product->setPhysicsState(isSleeping);
    }
}
