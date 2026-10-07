
#include "DeliveryBoxPhysicsController.h"

#include "DeliveryBox.h"
#include "DeliveryBoxInteraction.h"
#include "../interaction/InteractionController.h"
#include "../physics/PhysicsSystem.h"
#include "../scene/StoreScene.h"

#include <iostream>

DeliveryBoxPhysicsController::DeliveryBoxPhysicsController(PhysicsSystem& physics,
                                                           StoreScene& storeScene)
    : m_physics(physics),
      m_storeScene(storeScene),
      m_bodyPhysicsIds(),
      m_backLidPhysicsId(0),
      m_frontLidPhysicsId(0),
      m_transportPhysicsId(0)
{
}

DeliveryBoxPhysicsController::~DeliveryBoxPhysicsController()
{
    removeOpenBodies();
    removeTransportBody();
}

void DeliveryBoxPhysicsController::resetToOpenBodies()
{
    removeTransportBody();
    removeOpenBodies();

    createOpenBodies();
}

void DeliveryBoxPhysicsController::switchToTransportBody()
{
    // Nach dem Versiegeln werden sechs kinematische Einzelteile durch einen einzigen dynamischen Transportkörper ersetzt
    removeOpenBodies();
    removeTransportBody();

    createTransportBody();
}

void DeliveryBoxPhysicsController::updateBeforePhysics()
{
    DeliveryBox& deliveryBox = m_storeScene.deliveryBox();

    // Beim Tragen folgt der Physikkörper dem Carry-Transform und darf nicht selbst simuliert werden
    if (m_transportPhysicsId == 0 || !deliveryBox.isBeingCarried())
    {
        return;
    }

    m_physics.updateBodyTransform(m_transportPhysicsId, transportTransform());
}

void DeliveryBoxPhysicsController::updateAfterPhysics()
{
    const DeliveryBox& deliveryBox = m_storeScene.deliveryBox();

    // Nach dem Ablegen ist die Physik wieder führend und bewegt den sichtbaren Karton
    if (m_transportPhysicsId == 0 || deliveryBox.isBeingCarried())
    {
        return;
    }

    synchronizeDeliveryBoxFromPhysics();
}

void DeliveryBoxPhysicsController::updateOpenBodies()
{
    const std::array<Matrix, 5>& bodyTransforms = m_storeScene.deliveryBoxBodyPhysicsTransforms();

    // Offener Karton: Boden, vier Wände und beide Deckel bleiben kinematisch und folgen der Deckelanimation
    for (std::size_t index = 0; index < bodyTransforms.size(); ++index)
    {
        if (m_bodyPhysicsIds[index] == 0)
        {
            continue;
        }

        m_physics.updateBodyTransform(m_bodyPhysicsIds[index], bodyTransforms[index]);
    }

    if (m_backLidPhysicsId != 0)
    {
        m_physics.updateBodyTransform(m_backLidPhysicsId,
                                      m_storeScene.deliveryBoxBackLidPhysicsTransform());
    }

    if (m_frontLidPhysicsId != 0)
    {
        m_physics.updateBodyTransform(m_frontLidPhysicsId,
                                      m_storeScene.deliveryBoxFrontLidPhysicsTransform());
    }
}

void DeliveryBoxPhysicsController::handleCarryAction(const CarryAction& carryAction)
{
    if (carryAction.object == nullptr || m_transportPhysicsId == 0)
    {
        return;
    }

    DeliveryBoxInteraction* deliveryBoxInteraction =
        dynamic_cast<DeliveryBoxInteraction*>(carryAction.object);

    if (deliveryBoxInteraction == nullptr)
    {
        return;
    }

    if (carryAction.type == CarryActionType::PickedUp)
    {
        m_physics.startCarryingBody(m_transportPhysicsId, transportTransform());

        return;
    }

    if (carryAction.type == CarryActionType::Dropped)
    {
        m_physics.releaseBody(m_transportPhysicsId, transportTransform());
    }
}

void DeliveryBoxPhysicsController::createOpenBodies()
{
    const std::array<Matrix, 5>& bodyTransforms = m_storeScene.deliveryBoxBodyPhysicsTransforms();

    const std::array<Vector, 5>& bodySizes = m_storeScene.deliveryBoxBodyPhysicsSizes();

    for (std::size_t index = 0; index < bodyTransforms.size(); ++index)
    {
        m_bodyPhysicsIds[index] =
            m_physics.addKinematicBox(bodyTransforms[index], bodySizes[index]);
    }

    m_backLidPhysicsId =
        m_physics.addKinematicBox(m_storeScene.deliveryBoxBackLidPhysicsTransform(),
                                  m_storeScene.deliveryBoxLidPhysicsSize());

    m_frontLidPhysicsId =
        m_physics.addKinematicBox(m_storeScene.deliveryBoxFrontLidPhysicsTransform(),
                                  m_storeScene.deliveryBoxLidPhysicsSize());
}

void DeliveryBoxPhysicsController::removeOpenBodies()
{
    for (PhysicsBodyId& bodyId : m_bodyPhysicsIds)
    {
        m_physics.removeBody(bodyId);

        bodyId = 0;
    }

    m_physics.removeBody(m_backLidPhysicsId);

    m_backLidPhysicsId = 0;

    m_physics.removeBody(m_frontLidPhysicsId);

    m_frontLidPhysicsId = 0;
}

void DeliveryBoxPhysicsController::createTransportBody()
{
    const DeliveryBox& deliveryBox = m_storeScene.deliveryBox();

    Vector colliderSize = deliveryBox.geometry().outerBounds.size() * deliveryBox.worldScale();

    const float deliveryBoxMass = 8.0f;

    m_transportPhysicsId =
        m_physics.addDynamicBox(transportTransform(), colliderSize, deliveryBoxMass);

    if (m_transportPhysicsId == 0)
    {
        std::cout << "Transport-Physikbody des Kartons konnte nicht erstellt werden" << std::endl;
    }
}

void DeliveryBoxPhysicsController::removeTransportBody()
{
    m_physics.removeBody(m_transportPhysicsId);

    m_transportPhysicsId = 0;
}

Matrix DeliveryBoxPhysicsController::transportTransform() const
{
    const DeliveryBox& deliveryBox = m_storeScene.deliveryBox();

    Vector localCenter = deliveryBox.geometry().outerBounds.center() * deliveryBox.worldScale();

    Matrix centerOffsetTransform;

    centerOffsetTransform.translation(localCenter);

    return deliveryBox.worldTransform() * centerOffsetTransform;
}

void DeliveryBoxPhysicsController::synchronizeDeliveryBoxFromPhysics()
{
    if (m_transportPhysicsId == 0)
    {
        return;
    }

    Matrix physicsTransform = m_physics.getBodyTransform(m_transportPhysicsId);

    const DeliveryBox& deliveryBox = m_storeScene.deliveryBox();

    Vector localCenter = deliveryBox.geometry().outerBounds.center() * deliveryBox.worldScale();

    Matrix inverseCenterOffset;

    inverseCenterOffset.translation(-localCenter);

    // Der Physikkörper liegt im Mittelpunkt des Kartons, das sichtbare Asset verwendet dagegen seinen lokalen Ursprung
    Matrix deliveryBoxWorldTransform = physicsTransform * inverseCenterOffset;

    m_storeScene.deliveryBox().setWorldTransform(deliveryBoxWorldTransform);
}
