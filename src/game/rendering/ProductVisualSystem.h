#pragma once

#include "../../engine/vector.h"

#include <list>
#include <unordered_set>
#include <vector>

class AssetLoader;
class BaseModel;
class Product;
class Texture;

// Erzeugt und aktualisiert Produktmodelle getrennt von deren Gameplay- und Physikzustand
class ProductVisualSystem
{
public:
    explicit ProductVisualSystem(AssetLoader& assetLoader);

    void createVisual(Product* product, std::list<BaseModel*>& models);

    void update();

    void removeVisuals(const std::unordered_set<Product*>& products, std::list<BaseModel*>& models);

    void clear(std::list<BaseModel*>& models);

    BaseModel* modelFor(const Product* product) const;

private:
    // Verbindet ein Gameplay-Produkt mit Modell, Shader, Material und Visual-Transform
    struct ProductVisual
    {
        Product* product = nullptr;
        BaseModel* model = nullptr;
        BaseModel* barcodeModel = nullptr;

        Vector localOffset;
        Vector barcodeLocalPosition;

        float visualScale = 1.0f;
    };

    bool hasVisual(const Product* product) const;

    void updateVisualTransform(ProductVisual& visual) const;

    void removeVisual(ProductVisual& visual, std::list<BaseModel*>& models);

    AssetLoader& m_assetLoader;

    const Texture* m_barcodeTexture;

    std::vector<ProductVisual> m_visuals;
};
