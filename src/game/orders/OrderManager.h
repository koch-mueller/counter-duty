#pragma once

#include "Orders.h"

#include <cstddef>
#include <memory>
#include <vector>

// Verwaltet Reihenfolge und Zustandswechsel der Kundenaufträge
class OrderManager
{
public:
    OrderManager();

    void clear();

    void setOrders(const std::vector<Order>& orders);

    bool prepareNextOrder();

    bool acceptCurrentOrder();

    bool hasWaitingOrder() const;
    bool hasActiveOrder() const;

    const Order* currentOrder() const;
    const Order* activeOrder() const;

    bool registerScannedProduct(ProductType type);

    bool isProductStillRequired(ProductType type) const;

    bool markCurrentOrderDelivered(int deliveredOrderId);

    bool finishDeliveredOrder();

private:
    std::vector<Order> m_orders;

    std::unique_ptr<Order> m_currentOrder;

    std::size_t m_nextOrderIndex;
};
