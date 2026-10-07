#pragma once

#include "ProductDefinition.h"

#include <vector>

class AssetLoader;

// Hält die festen Produktdefinitionen und stellt sie über Typ oder ID bereit
class ProductCatalog
{
public:
    explicit ProductCatalog(const AssetLoader& assetLoader);

    const ProductDefinition* find(ProductType type) const;

private:
    void addDefinition(const AssetLoader& assetLoader,
                       ProductType type,
                       const std::string& name,
                       const std::string& modelPath,
                       float modelScale,
                       BarcodeSide barcodeSide,
                       float barcodeVisualScale);

    std::vector<ProductDefinition> m_definitions;
};
