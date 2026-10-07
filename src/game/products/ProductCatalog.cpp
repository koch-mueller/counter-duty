#include "ProductCatalog.h"

#include "../../assets/AssetLoader.h"
#include "../../engine/Aabb.h"

#include <iostream>

ProductCatalog::ProductCatalog(const AssetLoader& assetLoader)
{
    m_definitions.reserve(8);

    addDefinition(assetLoader,
                  ProductType::Cereal,
                  "Muesli",
                  "models/products/cereal/cereal.obj",
                  1.0f,
                  BarcodeSide::Bottom,
                  1.0f);

    addDefinition(assetLoader,
                  ProductType::SodaCan,
                  "Getraenkedose",
                  "models/products/soda_can/soda_can.obj",
                  1.2f,
                  BarcodeSide::Bottom,
                  1.0f);

    addDefinition(assetLoader,
                  ProductType::Detergent,
                  "Waschmittel",
                  "models/products/detergent/detergent.obj",
                  1.0f,
                  BarcodeSide::Bottom,
                  0.6f);

    addDefinition(assetLoader,
                  ProductType::Ketchup,
                  "Ketchup",
                  "models/products/ketchup/ketchup.obj",
                  1.0f,
                  BarcodeSide::Bottom,
                  0.9f);

    addDefinition(assetLoader,
                  ProductType::Mustard,
                  "Senf",
                  "models/products/mustard/mustard.obj",
                  1.0f,
                  BarcodeSide::Bottom,
                  0.7f);

    addDefinition(assetLoader,
                  ProductType::PotatoChips,
                  "Chips",
                  "models/products/potato_chips/potato_chips.obj",
                  1.0f,
                  BarcodeSide::Back,
                  1.0f);

    addDefinition(assetLoader,
                  ProductType::Butter,
                  "Butter",
                  "models/products/butter/butter.obj",
                  1.0f,
                  BarcodeSide::Bottom,
                  0.9f);

    addDefinition(assetLoader,
                  ProductType::Tomato,
                  "Tomate",
                  "models/products/tomato/tomato.obj",
                  1.35f,
                  BarcodeSide::Bottom,
                  0.6f);
}

const ProductDefinition* ProductCatalog::find(ProductType type) const
{
    for (const ProductDefinition& definition : m_definitions)
    {
        if (definition.type() == type)
        {
            return &definition;
        }
    }

    return nullptr;
}

void ProductCatalog::addDefinition(const AssetLoader& assetLoader,
                                   ProductType type,
                                   const std::string& name,
                                   const std::string& modelPath,
                                   float modelScale,
                                   BarcodeSide barcodeSide,
                                   float barcodeVisualScale)
{
    AABB modelBounds;

    if (!assetLoader.loadModelBounds(modelPath, modelBounds))
    {
        std::cerr << "Failed to load product bounds: " << modelPath << "\n";

        return;
    }

    // Gameplay-Collider werden direkt aus den Modell-Bounds und demselben Modellmaßstab abgeleitet
    Vector boundsSize = modelBounds.size() * modelScale;

    m_definitions.emplace_back(type,
                               name,
                               modelPath,
                               modelScale,
                               boundsSize,
                               barcodeSide,
                               barcodeVisualScale);
}
