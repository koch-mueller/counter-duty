#pragma once

#include "../orders/OrderPackingEvaluator.h"

#include <list>

class BaseModel;
class CarrySystem;
class DeliveryBoxPhysicsController;
class FirstPersonCamera;
class ToastManager;
class InteractionSystem;
class OrderManager;
class PhysicsSystem;
class PlayerController;
class StoreScene;

// Koordiniert Abschluss, Transport und Ablieferung eines fertig gepackten Auftrags
class DeliveryController
{
public:
    DeliveryController(StoreScene& storeScene,
                       OrderManager& orderManager,
                       PhysicsSystem& physics,
                       DeliveryBoxPhysicsController& deliveryBoxPhysics,
                       InteractionSystem& interactionSystem,
                       CarrySystem& carrySystem,
                       ToastManager& toastmanager);

    void reset();

    void updateBeforeScene(const PlayerController& player, float deltaTime);

    void updateAfterScene();

    const OrderPackingResult& packingResult() const;
    
    bool canDeliverCurrentBox(const FirstPersonCamera& camera) const;

    bool requestDelivery(const FirstPersonCamera& camera,
                         std::list<BaseModel*>& models);

private:
    void handleLidToggle(const PlayerController& player);

    void handleDeliveryBoxEvents();

    bool completeCurrentDelivery(std::list<BaseModel*>& models);

    StoreScene& m_storeScene;
    OrderManager& m_orderManager;
    PhysicsSystem& m_physics;

    DeliveryBoxPhysicsController& m_deliveryBoxPhysics;

    InteractionSystem& m_interactionSystem;

    CarrySystem& m_carrySystem;
    ToastManager& m_toastManager;

    OrderPackingEvaluator m_orderPackingEvaluator;

    OrderPackingResult m_orderPackingResult;
};
