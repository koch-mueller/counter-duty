#pragma once

#include "../products/Product.h"
#include "ShelfSlot.h"

#include <vector>

// Zustand einer Regalreihe während Entnahme, Nachfüllen und Vorschieben
enum class ShelfProductRowState
{
    Ready,
    Advancing,
    WaitingForRefill,
    Refilling
};

// Verwaltet Slots und Produkte einer einzelnen Regalreihe inklusive Auto-Forward
class ShelfProductRow
{
public:
    ShelfProductRow(ProductType productType,
                    const std::vector<Vector>& slotPositions,
                    const Vector& spawnPosition,
                    float frontDirectionZ);

    const std::vector<ShelfSlot>& slots() const;

    bool addProduct(Product* product);

    Product* frontProduct() const;

    ProductType productType() const;

    bool needsRefillProduct() const;

    bool addRefillProduct(Product* product);
    
    bool consumeRefillCompletedEvent();

    void update(float deltaTime);

private:
    void prepareProduct(Product* product, const Vector& position);

    void startAdvancing();

    void deactivateAllProducts();

    void activateFrontProduct();

    bool moveProductTowards(Product* product, const Vector& targetPosition, float deltaTime);

    bool moveProductsToSlots(float deltaTime);

    void setProductPosition(Product* product, const Vector& position);

    float m_frontDirectionZ;

    ShelfProductRowState m_state;

    std::vector<ShelfSlot> m_slots;
    std::vector<Product*> m_products;

    ProductType m_productType;

    Vector m_spawnPosition;

    float m_moveSpeed;
    float m_refillDelay;
    float m_refillTimer;
    
    bool m_refillCompletedEvent;
};
