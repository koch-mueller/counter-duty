#pragma once

#include "../../engine/vector.h"

// Speichert Position, Größe und Reinigungsfortschritt eines aktiven Spills
class Spill
{
public:
    Spill();

    void configure(const Vector& size, float requiredCleaningTime);

    void activate(const Vector& position);

    void deactivate();

    bool isActive() const;

    const Vector& position() const;

    Vector currentSize() const;

    float cleaningProgress() const;

    bool contains(const Vector& worldPosition) const;

    bool addCleaningProgress(float deltaTime);

private:
    Vector m_size;

    Vector m_position;

    float m_requiredCleaningTime;

    float m_currentCleaningTime;

    bool m_active;
};
