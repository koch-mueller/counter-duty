#pragma once

#include "ShelfProductRow.h"

#include <vector>

// Fasst die Produktreihen eines Regalmodells zusammen
class Shelf
{
public:
    void addRow(ProductType productType,
                const std::vector<Vector>& slotPositions,
                const Vector& spawnPosition,
                float frontDirectionZ);

    std::vector<ShelfProductRow>& rows();

    const std::vector<ShelfProductRow>& rows() const;

    void update(float deltaTime);

private:
    std::vector<ShelfProductRow> m_rows;
};
