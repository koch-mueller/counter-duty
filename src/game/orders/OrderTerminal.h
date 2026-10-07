#pragma once

#include "../../engine/Aabb.h"
#include "../interaction/IInteractable.h"

#include <string>

class OrderManager;

// Interaktionsobjekt des Terminals zum Öffnen und Annehmen von Aufträgen
class OrderTerminal : public IInteractable
{
public:
    OrderTerminal(const AABB& bounds, OrderManager& orderManager);

    bool canInteract() const override;

    std::string getPromptText() const override;

    void interact() override;
    
    bool consumeOpenRequested();

    const AABB& getCollider() const override;

private:
    AABB m_bounds;
    OrderManager& m_orderManager;
    bool m_openRequested;
};
