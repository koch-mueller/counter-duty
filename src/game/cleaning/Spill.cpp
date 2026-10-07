#include "Spill.h"

Spill::Spill()
    : m_size(1.0f, 0.01f, 1.0f),
      m_position(0.0f, 0.0f, 0.0f),
      m_requiredCleaningTime(3.0f),
      m_currentCleaningTime(0.0f),
      m_active(false)
{
}

void Spill::configure(const Vector& size, float requiredCleaningTime)
{
    m_size = size;

    m_requiredCleaningTime = requiredCleaningTime;
}

void Spill::activate(const Vector& position)
{
    m_position = position;

    m_currentCleaningTime = 0.0f;

    m_active = true;
}

void Spill::deactivate()
{
    m_currentCleaningTime = 0.0f;

    m_active = false;
}

bool Spill::isActive() const
{
    return m_active;
}

const Vector& Spill::position() const
{
    return m_position;
}

Vector Spill::currentSize() const
{
    return m_size;
}

float Spill::cleaningProgress() const
{
    if (m_requiredCleaningTime <= 0.0f)
    {
        return 1.0f;
    }

    // Der normierte Fortschritt wird auch direkt für das Ausblenden der Spill-Textur verwendet
    float progress = m_currentCleaningTime / m_requiredCleaningTime;

    if (progress > 1.0f)
    {
        progress = 1.0f;
    }

    return progress;
}

bool Spill::contains(const Vector& worldPosition) const
{
    if (!m_active)
    {
        return false;
    }

    Vector size = this->m_size;

    float halfWidth = size.X * 0.5f;

    float halfDepth = size.Z * 0.5f;

    // Für die Reinigung zählt nur die Fläche auf dem Boden; die Y-Koordinate ist deshalb absichtlich irrelevant
    return worldPosition.X >= m_position.X - halfWidth
           && worldPosition.X <= m_position.X + halfWidth
           && worldPosition.Z >= m_position.Z - halfDepth
           && worldPosition.Z <= m_position.Z + halfDepth;
}

bool Spill::addCleaningProgress(float deltaTime)
{
    if (!m_active || deltaTime <= 0.0f)
    {
        return false;
    }

    m_currentCleaningTime += deltaTime;

    if (m_currentCleaningTime < m_requiredCleaningTime)
    {
        return false;
    }

    // Das Erreichen der benötigten Zeit beendet den Spill sofort und signalisiert dem Aufrufer den Abschluss
    m_currentCleaningTime = m_requiredCleaningTime;

    m_active = false;

    return true;
}
