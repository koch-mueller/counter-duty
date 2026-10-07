#pragma once

#include "../physics/IPhysicsWorld.h"

#include <array>

class PhysicsSystem;
class StoreScene;
struct CarryAction;

// Synchronisiert die verschiedenen Physik-Körper des Kartons mit seinem Gameplay-Zustand
class DeliveryBoxPhysicsController
{
public:
    DeliveryBoxPhysicsController(PhysicsSystem& physics, StoreScene& storeScene);

    ~DeliveryBoxPhysicsController();

    void resetToOpenBodies();
    void switchToTransportBody();

    void updateBeforePhysics();
    void updateAfterPhysics();
    void updateOpenBodies();

    void handleCarryAction(const CarryAction& carryAction);

private:
    void createOpenBodies();
    void removeOpenBodies();

    void createTransportBody();
    void removeTransportBody();

    Matrix transportTransform() const;

    void synchronizeDeliveryBoxFromPhysics();

    PhysicsSystem& m_physics;
    StoreScene& m_storeScene;

    std::array<PhysicsBodyId, 5> m_bodyPhysicsIds;

    PhysicsBodyId m_backLidPhysicsId;
    PhysicsBodyId m_frontLidPhysicsId;
    PhysicsBodyId m_transportPhysicsId;
};
