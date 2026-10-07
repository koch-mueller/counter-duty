#include "Orders.h"

#include <algorithm>

OrderLine::OrderLine(const ProductDefinition* definition, int requiredAmount)
    : m_definition(definition),
      m_requiredAmount(std::max(0, requiredAmount)),
      m_scannedAmount(0)
{
}

const ProductDefinition* OrderLine::definition() const
{
    return m_definition;
}

int OrderLine::requiredAmount() const
{
    return m_requiredAmount;
}

int OrderLine::scannedAmount() const
{
    return m_scannedAmount;
}

int OrderLine::remainingAmount() const
{
    return std::max(0, m_requiredAmount - m_scannedAmount);
}

bool OrderLine::registerScan()
{
    if (m_definition == nullptr || m_scannedAmount >= m_requiredAmount)
    {
        return false;
    }

    ++m_scannedAmount;

    return true;
}

void OrderLine::reset()
{
    m_scannedAmount = 0;
}

Order::Order(int id, const std::vector<OrderLine>& lines)
    : m_id(id),
      m_lines(lines),
      m_state(OrderState::WaitingForAcceptance)
{
}

int Order::id() const
{
    return m_id;
}

OrderState Order::state() const
{
    return m_state;
}

const std::vector<OrderLine>& Order::lines() const
{
    return m_lines;
}

const OrderLine* Order::findLine(ProductType type) const
{
    for (const OrderLine& line : m_lines)
    {
        const ProductDefinition* definition = line.definition();

        if (definition != nullptr && definition->type() == type)
        {
            return &line;
        }
    }

    return nullptr;
}

OrderLine* Order::findLine(ProductType type)
{
    for (OrderLine& line : m_lines)
    {
        const ProductDefinition* definition = line.definition();

        if (definition != nullptr && definition->type() == type)
        {
            return &line;
        }
    }

    return nullptr;
}

bool Order::activate()
{
    if (m_state != OrderState::WaitingForAcceptance)
    {
        return false;
    }

    m_state = OrderState::Active;

    return true;
}

bool Order::registerScan(ProductType type)
{
    if (m_state != OrderState::Active)
    {
        return false;
    }

    OrderLine* line = findLine(type);

    if (line == nullptr)
    {
        return false;
    }

    return line->registerScan();
}

void Order::resetForWaiting()
{
    for (OrderLine& line : m_lines)
    {
        line.reset();
    }

    m_state = OrderState::WaitingForAcceptance;
}

bool Order::markDelivered()
{
    if (m_state != OrderState::Active)
    {
        return false;
    }

    m_state = OrderState::Delivered;

    return true;
}
