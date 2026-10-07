#pragma once

#include "Orders.h"

#include <vector>

class ProductCatalog;

// Erzeugt die vorgegebenen Aufträge auf Basis des Produktkatalogs
class OrderCatalog
{
public:
    explicit OrderCatalog(const ProductCatalog& productCatalog);

    const std::vector<Order>& orders() const;

private:
    std::vector<Order> m_orders;
};
