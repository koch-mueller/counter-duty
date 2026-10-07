#include "ProductDefinition.h"

namespace
{
    Vector calculateBarcodePosition(const Vector& boundsSize, BarcodeSide barcodeSide)
    {
        Vector halfSize = boundsSize * 0.5f;

        switch (barcodeSide)
        {
            case BarcodeSide::Bottom:
                return Vector(0.0f, -halfSize.Y, 0.0f);

            case BarcodeSide::Top:
                return Vector(0.0f, halfSize.Y, 0.0f);

            case BarcodeSide::Front:
                return Vector(0.0f, 0.0f, halfSize.Z);

            case BarcodeSide::Back:
                return Vector(0.0f, 0.0f, -halfSize.Z);

            case BarcodeSide::Left:
                return Vector(-halfSize.X, 0.0f, 0.0f);

            case BarcodeSide::Right:
                return Vector(halfSize.X, 0.0f, 0.0f);
        }

        return Vector(0.0f, -halfSize.Y, 0.0f);
    }

    Vector calculateBarcodeNormal(BarcodeSide barcodeSide)
    {
        switch (barcodeSide)
        {
            case BarcodeSide::Bottom:
                return Vector(0.0f, -1.0f, 0.0f);

            case BarcodeSide::Top:
                return Vector(0.0f, 1.0f, 0.0f);

            case BarcodeSide::Front:
                return Vector(0.0f, 0.0f, 1.0f);

            case BarcodeSide::Back:
                return Vector(0.0f, 0.0f, -1.0f);

            case BarcodeSide::Left:
                return Vector(-1.0f, 0.0f, 0.0f);

            case BarcodeSide::Right:
                return Vector(1.0f, 0.0f, 0.0f);
        }

        return Vector(0.0f, -1.0f, 0.0f);
    }
}

ProductDefinition::ProductDefinition(ProductType type,
                                     const std::string& name,
                                     const std::string& modelPath,
                                     float modelScale,
                                     const Vector& boundsSize,
                                     BarcodeSide barcodeSide,
                                     float barcodeVisualScale)
    : m_type(type),
      m_name(name),
      m_modelPath(modelPath),
      m_modelScale(modelScale),
      m_boundsSize(boundsSize),
      m_barcodeSide(barcodeSide),
      m_barcodePosition(calculateBarcodePosition(boundsSize, barcodeSide)),
      m_barcodeNormal(calculateBarcodeNormal(barcodeSide)),
      m_barcodeVisualScale(barcodeVisualScale)
{
}

ProductType ProductDefinition::type() const
{
    return m_type;
}

const std::string& ProductDefinition::name() const
{
    return m_name;
}

const std::string& ProductDefinition::modelPath() const
{
    return m_modelPath;
}

float ProductDefinition::modelScale() const
{
    return m_modelScale;
}

const Vector& ProductDefinition::boundsSize() const
{
    return m_boundsSize;
}

BarcodeSide ProductDefinition::barcodeSide() const
{
    return m_barcodeSide;
}

const Vector& ProductDefinition::barcodePosition() const
{
    return m_barcodePosition;
}

const Vector& ProductDefinition::barcodeNormal() const
{
    return m_barcodeNormal;
}

float ProductDefinition::barcodeVisualScale() const
{
    return m_barcodeVisualScale;
}
