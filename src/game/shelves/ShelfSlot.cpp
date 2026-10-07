#include "ShelfSlot.h"

ShelfSlot::ShelfSlot(const Vector& position)
    : m_position(position)
{
}

const Vector& ShelfSlot::position() const
{
    return m_position;
}
