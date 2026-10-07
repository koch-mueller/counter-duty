#pragma once

#include "IInteractable.h"
#include "../player/FirstPersonCamera.h"

#include <list>
#include <string>

// Bestimmt per Raycast das aktuell fokussierte Interaktionsobjekt und führt dessen Aktion aus
class InteractionSystem
{
public:
    explicit InteractionSystem(float maxInteractionDistance);

    void addInteractable(IInteractable* interactable);

    void removeInteractable(IInteractable* interactable);

    void clear();

    void update(const FirstPersonCamera& camera);

    void interact();
    void secondaryInteract();

    void setBlockingColliders(const std::list<AABB>& colliders);

    IInteractable* focusedInteractable() const;

    const std::list<IInteractable*>& interactables() const;

    std::string interactionPrompt() const;

private:
    std::list<IInteractable*> m_interactables;

    IInteractable* m_focusedInteractable;

    float m_maxInteractionDistance;

    const std::list<AABB>* m_blockingColliders;
};
