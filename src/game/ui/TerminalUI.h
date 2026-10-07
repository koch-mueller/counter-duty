#pragma once

class Order;
class OrderManager;

// Verwaltet Öffnen, Auswahl und Annahme von Aufträgen im Terminal-Overlay
class TerminalUI
{
public:
    explicit TerminalUI(OrderManager& orderManager);

    void open();

    void close();

    bool isOpen() const;

    void update(bool confirmPressed);

    bool canAcceptOrder() const;

    const Order* order() const;

private:
    OrderManager& m_orderManager;

    bool m_open;
};
