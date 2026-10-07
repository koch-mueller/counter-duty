#pragma once

#include "../orders/OrderPackingResult.h"

#include <string>
#include "../../engine/vector.h"

class Order;

// Für das HUD aufbereitete Daten einer Auftragsposition
struct HUDOrderLine
{
    std::string productName;

    int requiredAmount = 0;
    int packedAmount = 0;
    int scannedOutsideAmount = 0;
    int notScannedAmount = 0;
};

// Sammelt pro Frame alle Texte und Statusdaten, die das Gameplay-HUD darstellen soll
class HUDModel
{
public:
    HUDModel();

    void beginFrame();

    void updateOrder(const Order* activeOrder, const OrderPackingResult& packingResult);

    void clearOrder();

    void clear();

    void setContextPrompt(const std::string& prompt);
    void setCarryHint(const std::string& hint);
    void setScanHint(const std::string& hint);
    void setObjectiveText(const std::string& text);
    void setEventText(const std::string& text);

    bool hasActiveOrder() const;
    int orderId() const;

    const std::vector<HUDOrderLine>& orderLines() const;
    const std::vector<std::string>& boxWarnings() const;

    bool isOrderFullyPacked() const;

    const std::string& contextPrompt() const;
    const std::string& carryHint() const;
    const std::string& scanHint() const;
    const std::string& objectiveText() const;
    const std::string& eventText() const;

private:
    std::string createWarningText(const BoxWarning& warning) const;

    bool m_hasActiveOrder;
    int m_orderId;

    std::vector<HUDOrderLine> m_orderLines;
    std::vector<std::string> m_boxWarnings;

    bool m_orderFullyPacked;

    std::string m_contextPrompt;
    std::string m_carryHint;
    std::string m_scanHint;
    std::string m_objectiveText;
    std::string m_eventText;
};
