#include "OrderTerminal.h"

#include "OrderManager.h"

OrderTerminal::OrderTerminal(const AABB& bounds, OrderManager& orderManager)
    : m_bounds(bounds),
      m_orderManager(orderManager),
      m_openRequested(false)
{
}

bool OrderTerminal::canInteract() const
{
    return m_orderManager.hasWaitingOrder() || m_orderManager.hasActiveOrder();
}

std::string OrderTerminal::getPromptText() const
{
    return "[E] Terminal oeffnen";
}

void OrderTerminal::interact()
{
    if (!canInteract())
    {
        return;
    }

    m_openRequested = true;
}

const AABB& OrderTerminal::getCollider() const
{
    return m_bounds;
}

bool OrderTerminal::consumeOpenRequested()
{
    bool openRequested = m_openRequested;

    m_openRequested = false;

    return openRequested;
}
