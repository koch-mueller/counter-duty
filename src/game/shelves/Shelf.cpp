#include "Shelf.h"

void Shelf::addRow(ProductType productType,
                   const std::vector<Vector>& slotPositions,
                   const Vector& spawnPosition,
                   float frontDirectionZ)
{
    m_rows.emplace_back(productType, slotPositions, spawnPosition, frontDirectionZ);
}

std::vector<ShelfProductRow>& Shelf::rows()
{
    return m_rows;
}

const std::vector<ShelfProductRow>& Shelf::rows() const
{
    return m_rows;
}

void Shelf::update(float deltaTime)
{
    for (ShelfProductRow& row : m_rows)
    {
        row.update(deltaTime);
    }
}
