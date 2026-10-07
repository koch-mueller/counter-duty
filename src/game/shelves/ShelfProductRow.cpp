#include "ShelfProductRow.h"

namespace
{
    constexpr float kPi = 3.14159265358979323846f;

    constexpr float kDefaultMoveSpeed = 0.6f;

    constexpr float kDefaultRefillDelay = 0.75f;

    constexpr float kTargetTolerance = 0.001f;
}

ShelfProductRow::ShelfProductRow(ProductType productType,
                                 const std::vector<Vector>& slotPositions,
                                 const Vector& spawnPosition,
                                 float frontDirectionZ)
    : m_state(ShelfProductRowState::Ready),
      m_productType(productType),
      m_spawnPosition(spawnPosition),
      m_frontDirectionZ(frontDirectionZ),
      m_moveSpeed(kDefaultMoveSpeed),
      m_refillDelay(kDefaultRefillDelay),
      m_refillTimer(0.0f),
      m_refillCompletedEvent(false)
{
    m_slots.reserve(slotPositions.size());

    for (const Vector& slotPosition : slotPositions)
    {
        m_slots.emplace_back(slotPosition);
    }
}

const std::vector<ShelfSlot>& ShelfProductRow::slots() const
{
    return m_slots;
}

bool ShelfProductRow::addProduct(Product* product)
{
    if (product == nullptr || m_products.size() >= m_slots.size())
    {
        return false;
    }

    const Vector& slotPosition = m_slots[m_products.size()].position();

    prepareProduct(product, slotPosition);

    m_products.push_back(product);

    return true;
}

Product* ShelfProductRow::frontProduct() const
{
    if (m_products.empty())
    {
        return nullptr;
    }

    return m_products.front();
}

ProductType ShelfProductRow::productType() const
{
    return m_productType;
}

bool ShelfProductRow::needsRefillProduct() const
{
    return m_state == ShelfProductRowState::WaitingForRefill && m_products.size() < m_slots.size();
}

bool ShelfProductRow::addRefillProduct(Product* product)
{
    if (product == nullptr || m_state != ShelfProductRowState::WaitingForRefill
        || m_products.size() >= m_slots.size())
    {
        return false;
    }

    prepareProduct(product, m_spawnPosition);

    m_products.push_back(product);

    m_state = ShelfProductRowState::Refilling;

    return true;
}

void ShelfProductRow::update(float deltaTime)
{
    // Die Reihe durchläuft Entnahme -> Vorschieben -> Nachfüllen -> Ready als kleinen Zustandsautomaten
    switch (m_state)
    {
        case ShelfProductRowState::Ready:
        {
            Product* product = frontProduct();

            if (product == nullptr)
            {
                startAdvancing();
                break;
            }

            // Sobald das vorderste Produkt aufgenommen wurde, wird es aus der Reihe entfernt und die restlichen rücken nach
            if (product->state() != ProductState::OnShelf)
            {
                m_products.erase(m_products.begin());

                startAdvancing();
                break;
            }

            activateFrontProduct();
            break;
        }

        case ShelfProductRowState::Advancing:
        {
            deactivateAllProducts();

            if (deltaTime > 0.0f)
            {
                m_refillTimer += deltaTime;
            }

            bool productsReachedSlots = moveProductsToSlots(deltaTime);

            bool refillDelayFinished = m_refillTimer >= m_refillDelay;

            if (productsReachedSlots && refillDelayFinished)
            {
                m_state = ShelfProductRowState::WaitingForRefill;
            }

            break;
        }

        case ShelfProductRowState::WaitingForRefill:
        {
            deactivateAllProducts();
            break;
        }

        case ShelfProductRowState::Refilling:
        {
            deactivateAllProducts();

            if (!moveProductsToSlots(deltaTime))
            {
                break;
            }

            m_refillTimer = 0.0f;

            m_state = ShelfProductRowState::Ready;
            
            m_refillCompletedEvent = true;

            activateFrontProduct();
            break;
        }
    }
}

bool ShelfProductRow::consumeRefillCompletedEvent()
{
    bool refillCompleted = m_refillCompletedEvent;

    m_refillCompletedEvent = false;

    return refillCompleted;
}

void ShelfProductRow::prepareProduct(Product* product, const Vector& position)
{
    if (product == nullptr)
    {
        return;
    }

    setProductPosition(product, position);

    product->setState(ProductState::OnShelf);

    product->setInteractable(false);
}

void ShelfProductRow::startAdvancing()
{
    // Während des Vorschubs kann kein Produkt aus derselben Reihe aufgenommen werden
    deactivateAllProducts();

    m_refillTimer = 0.0f;

    m_state = ShelfProductRowState::Advancing;
}

void ShelfProductRow::deactivateAllProducts()
{
    for (Product* product : m_products)
    {
        if (product == nullptr)
        {
            continue;
        }

        product->setInteractable(false);
    }
}

void ShelfProductRow::activateFrontProduct()
{
    deactivateAllProducts();

    // Nur das vorderste Produkt darf interaktiv sein, hintere Produkte werden erst nach dem Vorschieben freigegeben
    Product* product = frontProduct();

    if (product == nullptr || product->state() != ProductState::OnShelf)
    {
        return;
    }

    product->setInteractable(true);
}

bool ShelfProductRow::moveProductTowards(Product* product,
                                         const Vector& targetPosition,
                                         float deltaTime)
{
    if (product == nullptr)
    {
        return true;
    }

    Vector currentPosition = product->position();

    Vector movement = targetPosition - currentPosition;

    float distance = movement.length();

    if (distance <= kTargetTolerance)
    {
        setProductPosition(product, targetPosition);

        return true;
    }

    if (deltaTime <= 0.0f)
    {
        return false;
    }

    // Positionsänderung ist zeitbasiert und wird am Ziel geklemmt, damit kein Produkt darüber hinausschießt
    float movementDistance = m_moveSpeed * deltaTime;

    if (distance <= movementDistance)
    {
        setProductPosition(product, targetPosition);

        return true;
    }

    movement.normalize();

    setProductPosition(product, currentPosition + movement * movementDistance);

    return false;
}

bool ShelfProductRow::moveProductsToSlots(float deltaTime)
{
    if (m_products.size() > m_slots.size())
    {
        return false;
    }

    bool allProductsReachedTarget = true;

    for (std::size_t index = 0; index < m_products.size(); ++index)
    {
        bool productReachedTarget =
            moveProductTowards(m_products[index], m_slots[index].position(), deltaTime);

        if (!productReachedTarget)
        {
            allProductsReachedTarget = false;
        }
    }

    return allProductsReachedTarget;
}

void ShelfProductRow::setProductPosition(Product* product, const Vector& position)
{
    if (product == nullptr)
    {
        return;
    }

    Matrix translationTransform;

    translationTransform.translation(position);

    if (m_frontDirectionZ < 0.0f)
    {
        Matrix rotationTransform;

        rotationTransform.rotationY(kPi);

        product->setWorldTransform(translationTransform * rotationTransform);

        return;
    }

    product->setWorldTransform(translationTransform);
}
