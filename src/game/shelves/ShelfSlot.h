#pragma once

#include "../../engine/vector.h"

// Ein fester Platz einer Regalreihe mit Welttransform und optionalem Produkt
class ShelfSlot
{
public:
    explicit ShelfSlot(const Vector& position);

    const Vector& position() const;

private:
    Vector m_position;
};
