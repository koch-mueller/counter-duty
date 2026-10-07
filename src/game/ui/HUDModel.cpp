#include "./HUDModel.h"

#include "../orders/Orders.h"

#include <cstddef>

HUDModel::HUDModel()
    : m_hasActiveOrder(false),
      m_orderId(0),
      m_orderFullyPacked(false)
{}

// Kurzlebige Hinweise werden pro Frame neu aufgebaut, der Auftragsstatus bleibt dagegen erhalten
void HUDModel::beginFrame()
{
    m_contextPrompt.clear();

    m_carryHint.clear();

    m_scanHint.clear();

    m_objectiveText.clear();

    m_eventText.clear();
}

void HUDModel::updateOrder(const Order* activeOrder,
                           const OrderPackingResult& packingResult)
{
    if (activeOrder == nullptr || activeOrder->state() != OrderState::Active)
    {
        clearOrder();

        return;
    }

    m_hasActiveOrder = true;

    m_orderId = activeOrder->id();

    m_orderLines.clear();

    m_boxWarnings.clear();

    const std::vector<OrderLine>& lines = activeOrder->lines();

    m_orderLines.reserve(lines.size());

    // Das HUD übernimmt nur darstellungsrelevante Werte aus Order und PackingResult
    // Dadurch kennt der Renderer selbst keine Gameplay-Regeln
    for (std::size_t index = 0; index < lines.size(); ++index)
    {
        const OrderLine& line = lines[index];

        if (line.definition() == nullptr)
        {
            continue;
        }

        OrderLinePackingStatus status;

        status.requiredAmount = line.requiredAmount();
        status.notScannedAmount = line.requiredAmount();

        if (index < packingResult.lineStatuses.size())
        {
            status = packingResult.lineStatuses[index];
        }

        HUDOrderLine hudLine;

        hudLine.productName = line.definition()->name();
        hudLine.requiredAmount = status.requiredAmount;
        hudLine.packedAmount = status.packedAmount;
        hudLine.scannedOutsideAmount = status.scannedOutsideAmount;
        hudLine.notScannedAmount = status.notScannedAmount;

        m_orderLines.push_back(hudLine);
    }

    for (const BoxWarning& warning : packingResult.warnings)
    {
        std::string warningText = createWarningText(warning);

        if (!warningText.empty())
        {
            m_boxWarnings.push_back(warningText);
        }
    }

    m_orderFullyPacked = packingResult.isComplete;
}

void HUDModel::clearOrder()
{
    m_hasActiveOrder = false;

    m_orderId = 0;

    m_orderLines.clear();

    m_boxWarnings.clear();

    m_orderFullyPacked = false;
}

void HUDModel::clear()
{
    clearOrder();

    beginFrame();
}

void HUDModel::setContextPrompt(const std::string& prompt)
{
    m_contextPrompt = prompt;
}

void HUDModel::setCarryHint(const std::string& hint)
{
    m_carryHint = hint;
}

void HUDModel::setScanHint(const std::string& hint)
{
    m_scanHint = hint;
}

void HUDModel::setObjectiveText(const std::string& text)
{
    m_objectiveText = text;
}

void HUDModel::setEventText(const std::string& text)
{
    m_eventText = text;
}

bool HUDModel::hasActiveOrder() const
{
    return m_hasActiveOrder;
}

int HUDModel::orderId() const
{
    return m_orderId;
}

const std::vector<HUDOrderLine>& HUDModel::orderLines() const
{
    return m_orderLines;
}

const std::vector<std::string>& HUDModel::boxWarnings() const
{
    return m_boxWarnings;
}

bool HUDModel::isOrderFullyPacked() const
{
    return m_orderFullyPacked;
}

const std::string& HUDModel::contextPrompt() const
{
    return m_contextPrompt;
}

const std::string& HUDModel::carryHint() const
{
    return m_carryHint;
}

const std::string& HUDModel::scanHint() const
{
    return m_scanHint;
}

const std::string& HUDModel::objectiveText() const
{
    return m_objectiveText;
}

const std::string& HUDModel::eventText() const
{
    return m_eventText;
}

// Übersetzt fachliche Packwarnungen an einer zentralen Stelle in kurze HUD-Texte
std::string HUDModel::createWarningText(const BoxWarning& warning) const
{
    if (warning.definition == nullptr || warning.amount <= 0)
    {
        return "";
    }

    switch (warning.type)
    {
        case BoxWarningType::Unscanned:
            return "[!] " + std::to_string(warning.amount) + " ungescannt im Karton: "
                   + warning.definition->name();

        case BoxWarningType::WrongProduct:
            return "[!] " + std::to_string(warning.amount) + " falsche Produkte im Karton: "
                   + warning.definition->name();

        case BoxWarningType::ExcessProduct:
            return "[!] " + std::to_string(warning.amount) + " zu viele im Karton: "
                   + warning.definition->name();
    }

    return "";
}
