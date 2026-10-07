
#include "OrderPackingEvaluator.h"

#include "../delivery/DeliveryBox.h"
#include "../products/Product.h"

#include <algorithm>
#include <cstddef>

OrderPackingResult OrderPackingEvaluator::evaluate(const Order* activeOrder,
                                                   const DeliveryBox& deliveryBox,
                                                   const std::vector<Product*>& products) const
{
    OrderPackingResult result;

    if (activeOrder == nullptr || activeOrder->state() != OrderState::Active)
    {
        return result;
    }

    result.lineStatuses.reserve(activeOrder->lines().size());

    // Für jede OrderLine werden gescannte Produkte getrennt nach "im Karton" und "außerhalb" gezählt
    for (const OrderLine& line : activeOrder->lines())
    {
        result.lineStatuses.push_back(
            evaluateLineStatus(line, *activeOrder, deliveryBox, products));
    }

    collectProductWarnings(*activeOrder, deliveryBox, products, result.warnings);

    collectExcessWarnings(*activeOrder, result.lineStatuses, result.warnings);

    result.isComplete = isComplete(result);

    return result;
}

// Bewertet eine einzelne Auftragszeile getrennt nach gescannten Produkten im Karton, ausserhalb und noch fehlenden Exemplaren
OrderLinePackingStatus OrderPackingEvaluator::evaluateLineStatus(
    const OrderLine& line,
    const Order& activeOrder,
    const DeliveryBox& deliveryBox,
    const std::vector<Product*>& products) const
{
    OrderLinePackingStatus status;

    const ProductDefinition* definition = line.definition();

    if (definition == nullptr)
    {
        return status;
    }

    status.requiredAmount = line.requiredAmount();

    ProductType productType = definition->type();

    for (const Product* product : products)
    {
        if (product == nullptr || product->definition().type() != productType
            || !product->isScanned() || product->scannedOrderId() != activeOrder.id())
        {
            continue;
        }

        bool isInsideBox = deliveryBox.containsProductId(static_cast<int>(product->id()));

        if (isInsideBox)
        {
            ++status.packedAmount;
        }
        else
        {
            ++status.scannedOutsideAmount;
        }
    }

    status.notScannedAmount =
        std::max(0, status.requiredAmount - status.packedAmount - status.scannedOutsideAmount);

    return status;
}

void OrderPackingEvaluator::collectProductWarnings(const Order& activeOrder,
                                                   const DeliveryBox& deliveryBox,
                                                   const std::vector<Product*>& products,
                                                   std::vector<BoxWarning>& warnings) const
{
    for (const Product* product : products)
    {
        if (product == nullptr)
        {
            continue;
        }

        bool isInsideBox = deliveryBox.containsProductId(static_cast<int>(product->id()));

        // Auch nur teilweise hineinragende Produkte erzeugen Warnungen und dürfen das Schließen nicht umgehen
        bool overlapsInnerArea = deliveryBox.overlapsInnerArea(product->localColliderVertices(),
                                                               product->getWorldTransform());

        if (!isInsideBox && !overlapsInnerArea)
        {
            continue;
        }

        const ProductDefinition& definition = product->definition();

        const OrderLine* orderLine = activeOrder.findLine(definition.type());

        if (orderLine == nullptr)
        {
            addWarning(warnings, BoxWarningType::WrongProduct, &definition);

            continue;
        }

        bool belongsToActiveOrder =
            product->isScanned() && product->scannedOrderId() == activeOrder.id();

        if (!belongsToActiveOrder)
        {
            addWarning(warnings, BoxWarningType::Unscanned, &definition);
        }
    }
}

void OrderPackingEvaluator::collectExcessWarnings(
    const Order& activeOrder,
    const std::vector<OrderLinePackingStatus>& lineStatuses,
    std::vector<BoxWarning>& warnings) const
{
    const std::vector<OrderLine>& lines = activeOrder.lines();

    std::size_t lineCount = std::min(lines.size(), lineStatuses.size());

    for (std::size_t index = 0; index < lineCount; ++index)
    {
        const OrderLine& line = lines[index];

        if (line.definition() == nullptr)
        {
            continue;
        }

        int excessAmount = lineStatuses[index].packedAmount - line.requiredAmount();

        if (excessAmount <= 0)
        {
            continue;
        }

        addWarning(warnings, BoxWarningType::ExcessProduct, line.definition(), excessAmount);
    }
}

void OrderPackingEvaluator::addWarning(std::vector<BoxWarning>& warnings,
                                       BoxWarningType type,
                                       const ProductDefinition* definition,
                                       int amount) const
{
    if (definition == nullptr || amount <= 0)
    {
        return;
    }

    // Gleiche Warnungen werden zusammengefasst, damit das HUD z. B. "2x falsches Produkt" anzeigen kann
    for (BoxWarning& warning : warnings)
    {
        bool sameWarning = warning.type == type && warning.definition == definition;

        if (!sameWarning)
        {
            continue;
        }

        warning.amount += amount;

        return;
    }

    BoxWarning warning;

    warning.type = type;

    warning.definition = definition;

    warning.amount = amount;

    warnings.push_back(warning);
}

bool OrderPackingEvaluator::isComplete(const OrderPackingResult& result) const
{
    // Vollständig bedeutet exakt gepackt: keine Warnung, nichts fehlt und kein gescanntes Produkt liegt außerhalb
    if (result.lineStatuses.empty() || !result.warnings.empty())
    {
        return false;
    }

    for (const OrderLinePackingStatus& status : result.lineStatuses)
    {
        bool lineComplete = status.requiredAmount > 0
                            && status.packedAmount == status.requiredAmount
                            && status.scannedOutsideAmount == 0 && status.notScannedAmount == 0;

        if (!lineComplete)
        {
            return false;
        }
    }

    return true;
}
