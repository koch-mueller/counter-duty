#include "./TerminalUI.h"

#include "../orders/OrderManager.h"

TerminalUI::TerminalUI(OrderManager& orderManager)
    : m_orderManager(orderManager),
      m_open(false)
{}

void TerminalUI::open()
{
    m_open = true;
}

void TerminalUI::close()
{
    m_open = false;
}

bool TerminalUI::isOpen() const
{
    return m_open;
}

// Das Terminal verändert nur den Auftragszustand; Darstellung und Eingabesperren liegen außerhalb
void TerminalUI::update(bool confirmPressed)
{
    if (!m_open || !confirmPressed)
    {
        return;
    }

    if (m_orderManager.hasWaitingOrder())
    {
        m_orderManager.acceptCurrentOrder();
    }
}

bool TerminalUI::canAcceptOrder() const
{
    return m_open && m_orderManager.hasWaitingOrder();
}

const Order* TerminalUI::order() const
{
    return m_orderManager.currentOrder();
}
