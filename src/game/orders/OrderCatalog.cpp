#include "OrderCatalog.h"

#include "../products/ProductCatalog.h"

namespace
{
    OrderLine createOrderLine(const ProductCatalog& productCatalog,
                              ProductType productType,
                              int requiredAmount)
    {
        return OrderLine(productCatalog.find(productType), requiredAmount);
    }
}

OrderCatalog::OrderCatalog(const ProductCatalog& productCatalog)
{
    m_orders.reserve(3);

    std::vector<OrderLine> firstOrderLines;

    firstOrderLines.push_back(createOrderLine(productCatalog, ProductType::Cereal, 1));

    firstOrderLines.push_back(createOrderLine(productCatalog, ProductType::Ketchup, 1));

    m_orders.emplace_back(1, firstOrderLines);

    std::vector<OrderLine> secondOrderLines;

    secondOrderLines.push_back(createOrderLine(productCatalog, ProductType::SodaCan, 2));

    secondOrderLines.push_back(createOrderLine(productCatalog, ProductType::PotatoChips, 1));

    m_orders.emplace_back(2, secondOrderLines);

    std::vector<OrderLine> thirdOrderLines;

    thirdOrderLines.push_back(createOrderLine(productCatalog, ProductType::Detergent, 1));

    thirdOrderLines.push_back(createOrderLine(productCatalog, ProductType::Mustard, 1));

    thirdOrderLines.push_back(createOrderLine(productCatalog, ProductType::Butter, 1));

    thirdOrderLines.push_back(createOrderLine(productCatalog, ProductType::Tomato, 1));

    m_orders.emplace_back(3, thirdOrderLines);
}

const std::vector<Order>& OrderCatalog::orders() const
{
    return m_orders;
}
