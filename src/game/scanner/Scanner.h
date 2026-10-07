#pragma once

#include "../../engine/Aabb.h"
#include "../../engine/vector.h"

// Prüft, ob der Barcode eines Produkts in der Scannerzone liegt und korrekt ausgerichtet ist
class Scanner
{
public:
    Scanner(const AABB& scanZone, const Vector& scannerNormal, float maximumScanAngleDegrees);

    bool containsBarcode(const Vector& barcodeWorldPosition) const;

    bool isBarcodeAngleValid(const Vector& barcodeWorldNormal) const;

    const AABB& scanZone() const;

private:
    AABB m_scanZone;
    Vector m_scannerNormal;

    float m_minimumAlignment;
};
