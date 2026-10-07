#include "Scanner.h"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float kPi = 3.14159265358979323846f;
}

Scanner::Scanner(const AABB& scanZone, const Vector& scannerNormal, float maximumScanAngleDegrees)
    : m_scanZone(scanZone),
      m_scannerNormal(scannerNormal),
      m_minimumAlignment(1.0f)
{
    if (m_scannerNormal.lengthSquared() > 0.000001f)
    {
        m_scannerNormal.normalize();
    }

    float clampedAngle = std::max(0.0f, std::min(maximumScanAngleDegrees, 180.0f));

    float angleRadians = clampedAngle * kPi / 180.0f;

    // Der Grenzwert bleibt konstant und muss nicht bei jeder Prüfung neu berechnet werden
    m_minimumAlignment = std::cos(angleRadians);
}

bool Scanner::containsBarcode(const Vector& barcodeWorldPosition) const
{
    return m_scanZone.contains(barcodeWorldPosition);
}

bool Scanner::isBarcodeAngleValid(const Vector& barcodeWorldNormal) const
{
    Vector normal = barcodeWorldNormal;

    if (normal.lengthSquared() <= 0.000001f)
    {
        return false;
    }

    normal.normalize();

    // Der Barcode muss dem Scanner entgegen zeigen. Der Dot-Product-Wert entspricht dabei dem Kosinus des Winkels
    float alignment = normal.dot(-m_scannerNormal);

    return alignment >= m_minimumAlignment;
}

const AABB& Scanner::scanZone() const
{
    return m_scanZone;
}
