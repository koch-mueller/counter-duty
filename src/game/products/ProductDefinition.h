#pragma once

#include "../../engine/vector.h"

#include <string>

// Eindeutige fachliche Produktart
enum class ProductType
{
    Cereal,
    SodaCan,
    Detergent,
    Ketchup,
    Mustard,
    PotatoChips,
    Butter,
    Tomato
};

// Seite des Modells, auf der der Barcode für den Scanner liegt
enum class BarcodeSide
{
    Bottom,
    Top,
    Front,
    Back,
    Left,
    Right
};

// Unveränderliche Stammdaten eines Produkttyps wie Name, Modell, Größe und Barcode
class ProductDefinition
{
public:
    ProductDefinition(ProductType type,
                      const std::string& name,
                      const std::string& modelPath,
                      float modelScale,
                      const Vector& boundsSize,
                      BarcodeSide barcodeSide,
                      float barcodeVisualScale);

    ProductType type() const;

    const std::string& name() const;

    const std::string& modelPath() const;

    float modelScale() const;

    const Vector& boundsSize() const;

    BarcodeSide barcodeSide() const;

    const Vector& barcodePosition() const;

    const Vector& barcodeNormal() const;

    float barcodeVisualScale() const;

private:
    ProductType m_type;

    std::string m_name;

    std::string m_modelPath;

    float m_modelScale;

    Vector m_boundsSize;

    BarcodeSide m_barcodeSide;

    Vector m_barcodePosition;

    Vector m_barcodeNormal;

    float m_barcodeVisualScale;
};
