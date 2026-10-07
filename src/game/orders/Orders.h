#pragma once

#include "../products/ProductDefinition.h"

#include <vector>

// Lebenszyklus eines Auftrags
enum class OrderState
{
    WaitingForAcceptance,
    Active,
    Delivered
};

// Eine benötigte Produktart mit Soll- und Scanmenge
class OrderLine
{
public:
    OrderLine(const ProductDefinition* definition, int requiredAmount);

    const ProductDefinition* definition() const;

    int requiredAmount() const;
    int scannedAmount() const;
    int remainingAmount() const;

    bool registerScan();

    void reset();

private:
    const ProductDefinition* m_definition;

    int m_requiredAmount;
    int m_scannedAmount;
};

// Ein vollständiger Auftrag mit mehreren Positionen und aktuellem Zustand
class Order
{
public:
    Order(int id, const std::vector<OrderLine>& lines);

    int id() const;

    OrderState state() const;

    const std::vector<OrderLine>& lines() const;

    const OrderLine* findLine(ProductType type) const;

    OrderLine* findLine(ProductType type);

    bool activate();

    bool registerScan(ProductType type);

    void resetForWaiting();

    bool markDelivered();

private:
    int m_id;

    std::vector<OrderLine> m_lines;

    OrderState m_state;
};
