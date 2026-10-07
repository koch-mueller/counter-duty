#pragma once

#include "OrderPackingResult.h"
#include "Orders.h"

#include <vector>

class DeliveryBox;
class Product;

// Vergleicht gescannte und im Karton enthaltene Produkte mit dem aktiven Auftrag
class OrderPackingEvaluator
{
public:
    OrderPackingResult evaluate(const Order* activeOrder,
                                const DeliveryBox& deliveryBox,
                                const std::vector<Product*>& products) const;

private:
    OrderLinePackingStatus evaluateLineStatus(const OrderLine& line,
                                              const Order& activeOrder,
                                              const DeliveryBox& deliveryBox,
                                              const std::vector<Product*>& products) const;

    void collectProductWarnings(const Order& activeOrder,
                                const DeliveryBox& deliveryBox,
                                const std::vector<Product*>& products,
                                std::vector<BoxWarning>& warnings) const;

    void collectExcessWarnings(const Order& activeOrder,
                               const std::vector<OrderLinePackingStatus>& lineStatuses,
                               std::vector<BoxWarning>& warnings) const;

    void addWarning(std::vector<BoxWarning>& warnings,
                    BoxWarningType type,
                    const ProductDefinition* definition,
                    int amount = 1) const;

    bool isComplete(const OrderPackingResult& result) const;
};
