#pragma once

#include "IPhysicsWorld.h"

#include <memory>
#include <unordered_map>

class Product;

// Verwaltet Physikkörper der Produkte und synchronisiert sie mit den Gameplay-Objekten
// Die Simulation läuft mit einem festen Zeitschritt
class PhysicsSystem
{
public:
    PhysicsSystem();
    ~PhysicsSystem();

    void update(float deltaTime);

    void updateCarriedProduct(Product* product);

    PhysicsBodyId addStaticBox(const Vector& center, const Vector& size);

    PhysicsBodyId addKinematicBox(const Matrix& transform, const Vector& size);

    PhysicsBodyId addDynamicBox(const Matrix& transform, const Vector& size, float mass);

    void startCarrying(Product* product);

    void releaseProduct(Product* product);

    void removeProduct(Product* product);

    void startCarryingBody(PhysicsBodyId bodyId, const Matrix& transform);

    void releaseBody(PhysicsBodyId bodyId, const Matrix& transform);

    Matrix getBodyTransform(PhysicsBodyId bodyId) const;

    void updateBodyTransform(PhysicsBodyId bodyId, const Matrix& transform);

    void removeBody(PhysicsBodyId bodyId);

private:
    PhysicsBodyId findProductBody(Product* product) const;

    PhysicsBodyId createProductBody(Product& product);

    void synchronizeProductTransforms();

    std::unique_ptr<IPhysicsWorld> m_world;

    float m_timeAccumulator;
    float m_fixedTimeStep;

    std::unordered_map<Product*, PhysicsBodyId> m_productBodies;
};
