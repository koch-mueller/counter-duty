#include "DeliveryController.h"

#include "DeliveryBox.h"
#include "DeliveryBoxInteraction.h"
#include "DeliveryBoxPhysicsController.h"

#include "../carry/CarrySystem.h"
#include "../interaction/InteractionSystem.h"
#include "../orders/OrderManager.h"
#include "../physics/PhysicsSystem.h"
#include "../player/FirstPersonCamera.h"
#include "../player/PlayerController.h"
#include "../products/Product.h"
#include "../ui/ToastManager.h"
#include "../scene/StoreScene.h"

DeliveryController::DeliveryController(StoreScene& storeScene,
                                       OrderManager& orderManager,
                                       PhysicsSystem& physics,
                                       DeliveryBoxPhysicsController& deliveryBoxPhysics,
                                       InteractionSystem& interactionSystem,
                                       CarrySystem& carrySystem,
                                       ToastManager& toastManager)
    : m_storeScene(storeScene),
      m_orderManager(orderManager),
      m_physics(physics),
      m_deliveryBoxPhysics(deliveryBoxPhysics),
      m_interactionSystem(interactionSystem),
      m_carrySystem(carrySystem),
      m_toastManager(toastManager)
{
}

void DeliveryController::reset()
{
    m_orderPackingResult = OrderPackingResult();
}

void DeliveryController::updateBeforeScene(const PlayerController& player, float deltaTime)
{
    DeliveryBox& deliveryBox = m_storeScene.deliveryBox();

    deliveryBox.updateContents(m_storeScene.products(), deltaTime);

    // Die Packprüfung wird jedes Frame neu berechnet, weil Produkte durch Physik noch in/aus dem Karton rutschen können
    m_orderPackingResult = m_orderPackingEvaluator.evaluate(m_orderManager.activeOrder(),
                                                            deliveryBox,
                                                            m_storeScene.products());

    if (deliveryBox.state() == DeliveryBoxState::Closing && !m_orderPackingResult.isComplete)
    {
        deliveryBox.blockClosing();

        m_toastManager.push("Kartoninhalt hat sich waehrend des Schliessens geaendert",
                                ToastType::Warning,
                                2.0f);

        return;
    }

    handleLidToggle(player);
}

void DeliveryController::updateAfterScene()
{
    // Die kinematischen Kartonteile müssen erst nach der visuellen Deckelanimation aktualisiert werden
    m_deliveryBoxPhysics.updateOpenBodies();

    handleDeliveryBoxEvents();
}

const OrderPackingResult& DeliveryController::packingResult() const
{
    return m_orderPackingResult;
}

void DeliveryController::handleLidToggle(const PlayerController& player)
{
    // F wirkt nur auf den aktuell fokussierten Karton, ein offener Karton darf erst bei vollstaendiger Packung schliessen
    if (!player.specialActionPressed())
    {
        return;
    }

    IInteractable* focusedInteractable = m_interactionSystem.focusedInteractable();

    DeliveryBoxInteraction* deliveryBoxInteraction =
        dynamic_cast<DeliveryBoxInteraction*>(focusedInteractable);

    if (deliveryBoxInteraction == nullptr)
    {
        return;
    }

    DeliveryBoxState boxState = m_storeScene.deliveryBox().state();

    if (boxState == DeliveryBoxState::Open)
    {
        if (m_orderPackingResult.isComplete)
        {
            m_interactionSystem.secondaryInteract();
        }
        else
        {
            m_toastManager.push("Karton ist noch nicht vollstaendig gepackt",
                                    ToastType::Warning,
                                    2.0f);
        }

        return;
    }

    if (boxState == DeliveryBoxState::Blocked)
    {
        m_interactionSystem.secondaryInteract();
    }
}

void DeliveryController::handleDeliveryBoxEvents()
{
    // Erst nach vollständig abgeschlossener Deckelanimation werden Inhalt und Physik auf Transportzustand umgestellt
    if (m_storeScene.consumeDeliveryBoxSealedEvent())
    {
        const Order* activeOrder = m_orderManager.activeOrder();

        if (activeOrder != nullptr)
        {
            bool boxSealed = m_storeScene.sealDeliveryBox(activeOrder->id());

            if (boxSealed)
            {
                m_deliveryBoxPhysics.switchToTransportBody();

                std::vector<Product*> sealedProducts = m_storeScene.sealedDeliveryBoxProducts();

                // Versiegelte Produkte folgen danach relativ zum Karton und benötigen keine eigenen Physikkörper mehr
                for (Product* product : sealedProducts)
                {
                    m_physics.removeProduct(product);
                }
            }
        }
    }

    if (m_storeScene.consumeDeliveryBoxBlockedEvent())
    {
        m_toastManager.push("Karton kann nicht geschlossen werden",
                                ToastType::Warning,
                                2.0f);
    }
}


bool DeliveryController::canDeliverCurrentBox(const FirstPersonCamera& camera) const
{
    if (m_carrySystem.isCarrying())
    {
        return false;
    }
    const DeliveryBox& deliveryBox = m_storeScene.deliveryBox();

    const Order* activeOrder = m_orderManager.activeOrder();

    if (activeOrder == nullptr)
    {
        return false;
    }

    if (deliveryBox.state() != DeliveryBoxState::Sealed)
    {
        return false;
    }

    if (deliveryBox.isBeingCarried())
    {
        return false;
    }

    if (deliveryBox.sealedOrderId() != activeOrder->id())
    {
        return false;
    }

    // Für die Abgabe müssen sowohl Spieler als auch kompletter Karton im vorgesehenen Lieferbereich sein
    if (!m_storeScene.isPlayerNearDeliveryZone(camera.position()))
    {
        return false;
    }

    return m_storeScene.isDeliveryBoxInDeliveryZone();
}

bool DeliveryController::requestDelivery(const FirstPersonCamera& camera,
                                         std::list<BaseModel*>& models)
{
    if (!canDeliverCurrentBox(camera))
    {
        return false;
    }

    return completeCurrentDelivery(models);
}

bool DeliveryController::completeCurrentDelivery(std::list<BaseModel*>& models)
{
    DeliveryBox& deliveryBox = m_storeScene.deliveryBox();

    int deliveredOrderId = deliveryBox.sealedOrderId();

    bool orderDelivered = m_orderManager.markCurrentOrderDelivered(deliveredOrderId);

    if (!orderDelivered)
    {
        return false;
    }

    // Nach erfolgreicher Lieferung werden alle zugehörigen Laufzeitobjekte aus Physik, Interaktion und Szene entfernt
    std::vector<Product*> deliveredProducts = m_storeScene.sealedDeliveryBoxProducts();

    for (Product* product : deliveredProducts)
    {
        m_physics.removeProduct(product);

        m_interactionSystem.removeInteractable(product);
    }

    m_storeScene.removeDeliveredProducts(deliveredProducts, models);

    m_storeScene.resetDeliveryBox();

    m_deliveryBoxPhysics.resetToOpenBodies();

    m_orderManager.finishDeliveredOrder();

    m_orderManager.prepareNextOrder();

    reset();

    m_toastManager.push("Lieferung erfolgreich abgeschlossen",
                        ToastType::Success,
                        2.5f);

    return true;
}
