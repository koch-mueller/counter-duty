#include "OrderManager.h"

OrderManager::OrderManager()
    : m_nextOrderIndex(0)
{
}

void OrderManager::clear()
{
    m_currentOrder.reset();

    m_orders.clear();

    m_nextOrderIndex = 0;
}

void OrderManager::setOrders(const std::vector<Order>& orders)
{
    m_currentOrder.reset();

    m_orders = orders;

    m_nextOrderIndex = 0;
}

bool OrderManager::prepareNextOrder()
{
    if (m_currentOrder != nullptr || m_orders.empty())
    {
        return false;
    }

    // Nach dem letzten Auftrag beginnt die kleine Auftragsliste wieder von vorne
    if (m_nextOrderIndex >= m_orders.size())
    {
        m_nextOrderIndex = 0;
    }

    m_currentOrder = std::make_unique<Order>(m_orders[m_nextOrderIndex]);

    m_currentOrder->resetForWaiting();

    ++m_nextOrderIndex;

    if (m_nextOrderIndex >= m_orders.size())
    {
        m_nextOrderIndex = 0;
    }

    return true;
}

bool OrderManager::acceptCurrentOrder()
{
    if (!hasWaitingOrder())
    {
        return false;
    }

    return m_currentOrder->activate();
}

bool OrderManager::hasWaitingOrder() const
{
    return m_currentOrder != nullptr && m_currentOrder->state() == OrderState::WaitingForAcceptance;
}

bool OrderManager::hasActiveOrder() const
{
    return m_currentOrder != nullptr && m_currentOrder->state() == OrderState::Active;
}

const Order* OrderManager::currentOrder() const
{
    return m_currentOrder.get();
}

const Order* OrderManager::activeOrder() const
{
    if (!hasActiveOrder())
    {
        return nullptr;
    }

    return m_currentOrder.get();
}

bool OrderManager::registerScannedProduct(ProductType type)
{
    if (!hasActiveOrder())
    {
        return false;
    }

    return m_currentOrder->registerScan(type);
}

bool OrderManager::isProductStillRequired(ProductType type) const
{
    const Order* order = activeOrder();

    if (order == nullptr)
    {
        return false;
    }

    const OrderLine* line = order->findLine(type);

    if (line == nullptr)
    {
        return false;
    }

    return line->remainingAmount() > 0;
}

bool OrderManager::markCurrentOrderDelivered(int deliveredOrderId)
{
    if (!hasActiveOrder() || m_currentOrder->id() != deliveredOrderId)
    {
        return false;
    }

    return m_currentOrder->markDelivered();
}

bool OrderManager::finishDeliveredOrder()
{
    if (m_currentOrder == nullptr || m_currentOrder->state() != OrderState::Delivered)
    {
        return false;
    }

    m_currentOrder.reset();

    return true;
}
