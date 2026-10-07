#pragma once

#include "../../engine/Aabb.h"

#include <string>
#include <vector>

// Gemeinsame Schnittstelle für alle Objekte, die durch den Interaktionsstrahl fokussiert werden können
class IInteractable
{
public:
    virtual ~IInteractable() = default;

    virtual bool canInteract() const = 0;

    virtual std::string getPromptText() const = 0;

    virtual void interact() = 0;

    virtual bool secondaryInteract()
    {
        return false;
    }

    virtual const AABB& getCollider() const = 0;

    virtual std::vector<AABB> getInteractionColliders() const
    {
        return {getCollider()};
    }

    virtual int interactionPriority() const
    {
        return 0;
    }
};
