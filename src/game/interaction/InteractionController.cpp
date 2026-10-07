#include "InteractionController.h"

#include "IInteractable.h"
#include "InteractionSystem.h"
#include "../carry/CarrySystem.h"
#include "../carry/ICarryable.h"
#include "../player/FirstPersonCamera.h"
#include "../player/PlayerController.h"
#include "../rendering/HighlightRenderer.h"

InteractionController::InteractionController(InteractionSystem& interactionSystem,
                                             CarrySystem& carrySystem,
                                             HighlightRenderer& highlighter)
    : m_interactionSystem(interactionSystem),
      m_carrySystem(carrySystem),
      m_highlighter(highlighter)
{
}

void InteractionController::update(const FirstPersonCamera& camera,
                                   const PlayerController& player,
                                   float deltaTime,
                                   bool blockInteraction)
{
    m_pendingCarryAction = CarryAction();

    m_interactionSystem.update(camera);

    if (player.interactPressed() && !blockInteraction)
    {
        handleInteraction(camera, player);
    }

    if (player.isObjectRotationActive())
    {
        m_carrySystem.rotateHeldObject(player.mouseDeltaX(), player.mouseDeltaY());
    }

    m_carrySystem.update(camera, deltaTime);

    updateInterface();
}

CarryAction InteractionController::consumeCarryAction()
{
    CarryAction action = m_pendingCarryAction;

    m_pendingCarryAction = CarryAction();

    return action;
}

const std::string& InteractionController::interactionPrompt() const
{
    return m_interactionPrompt;
}

const std::string& InteractionController::carryHint() const
{
    return m_carryHint;
}

void InteractionController::handleInteraction(const FirstPersonCamera& camera,
                                              const PlayerController& player)
{
    // E hat zwei Rollen: ohne gehaltenes Objekt wird fokussiert/interagiert, mit Objekt wird ein gültiger Drop versucht
    if (!m_carrySystem.isCarrying())
    {
        IInteractable* focusedInteractable = m_interactionSystem.focusedInteractable();

        if (focusedInteractable == nullptr)
        {
            return;
        }

        // Nicht tragbare Interaktionen bleiben im InteractionSystem; tragbare Objekte wechseln in das CarrySystem
        ICarryable* carryable = dynamic_cast<ICarryable*>(focusedInteractable);

        if (carryable == nullptr)
        {
            m_interactionSystem.interact();
            return;
        }

        if (!m_carrySystem.pickUpObject(carryable))
        {
            return;
        }

        m_pendingCarryAction.type = CarryActionType::PickedUp;

        m_pendingCarryAction.object = carryable;

        return;
    }

    ICarryable* heldObject = m_carrySystem.heldObject();

    if (!m_carrySystem.tryDropHeldObject(camera.position(), player.playerRadius()))
    {
        return;
    }

    m_pendingCarryAction.type = CarryActionType::Dropped;

    m_pendingCarryAction.object = heldObject;
}

void InteractionController::updateInterface()
{
    if (m_carrySystem.isCarrying())
    {
        m_highlighter.update(nullptr);

        m_interactionPrompt.clear();

        if (m_carrySystem.canRotateHeldObject())
        {
            m_carryHint = "[E] Ablegen | [RMB] Drehen";
        }
        else
        {
            m_carryHint = "[E] Ablegen";
        }

        return;
    }

    m_highlighter.update(m_interactionSystem.focusedInteractable());

    m_interactionPrompt = m_interactionSystem.interactionPrompt();

    m_carryHint.clear();
}
