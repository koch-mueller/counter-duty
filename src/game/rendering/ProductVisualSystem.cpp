#include "ProductVisualSystem.h"

#include "../../assets/AssetLoader.h"

#include "../../engine/Model.h"
#include "../../engine/PhongShader.h"
#include "../../engine/TexturedPlaneModel.h"
#include "../../engine/Texture.h"

#include "../products/Product.h"

#include <iostream>

namespace
{
    const char* kBarcodeTexturePath = "materials/barcode/barcode_diffuse.png";

    constexpr float kBarcodeSurfaceCoverage = 0.7f;

    constexpr float kBarcodeAspectRatio = 192.0f / 512.0f;

    constexpr float kBarcodeSurfaceOffset = 0.001f;

    TexturedPlaneModel* createBarcodeModel(const ProductDefinition& definition,
                                           const Texture* texture)
    {
        Vector boundsSize = definition.boundsSize();

        float surfaceWidth = 0.0f;

        float surfaceHeight = 0.0f;

        PlaneOrientation orientation = PlaneOrientation::XZ;

        bool flipNormal = false;

        // Abhängig von der Produktseite wird das Barcode-Quad in die passende lokale Ebene gelegt
        switch (definition.barcodeSide())
        {
            case BarcodeSide::Bottom:
                surfaceWidth = boundsSize.X;

                surfaceHeight = boundsSize.Z;

                orientation = PlaneOrientation::XZ;

                flipNormal = true;

                break;

            case BarcodeSide::Top:
                surfaceWidth = boundsSize.X;

                surfaceHeight = boundsSize.Z;

                orientation = PlaneOrientation::XZ;

                break;

            case BarcodeSide::Front:
                surfaceWidth = boundsSize.X;

                surfaceHeight = boundsSize.Y;

                orientation = PlaneOrientation::XY;

                break;

            case BarcodeSide::Back:
                surfaceWidth = boundsSize.X;

                surfaceHeight = boundsSize.Y;

                orientation = PlaneOrientation::XY;

                flipNormal = true;

                break;

            case BarcodeSide::Left:
                surfaceWidth = boundsSize.Z;

                surfaceHeight = boundsSize.Y;

                orientation = PlaneOrientation::YZ;

                flipNormal = true;

                break;

            case BarcodeSide::Right:
                surfaceWidth = boundsSize.Z;

                surfaceHeight = boundsSize.Y;

                orientation = PlaneOrientation::YZ;

                break;
        }

        float barcodeWidth =
            surfaceWidth * kBarcodeSurfaceCoverage * definition.barcodeVisualScale();

        float barcodeHeight = barcodeWidth * kBarcodeAspectRatio;

        float maximumHeight =
            surfaceHeight * kBarcodeSurfaceCoverage * definition.barcodeVisualScale();

        // Das Seitenverhältnis bleibt erhalten, auch wenn die gewählte Produktfläche zu niedrig ist
        if (barcodeHeight > maximumHeight)
        {
            barcodeHeight = maximumHeight;

            barcodeWidth = barcodeHeight / kBarcodeAspectRatio;
        }

        TexturedPlaneModel* barcodeModel = new TexturedPlaneModel(barcodeWidth,
                                                                  barcodeHeight,
                                                                  1.0f,
                                                                  1.0f,
                                                                  orientation,
                                                                  flipNormal);

        barcodeModel->shadowCaster(false);

        PhongShader* shader = new PhongShader();

        shader->diffuseColor(Color(1.0f, 1.0f, 1.0f));

        shader->ambientColor(Color(0.45f, 0.45f, 0.45f));

        shader->specularColor(Color(0.02f, 0.02f, 0.02f));

        shader->specularExp(4.0f);

        shader->diffuseTexture(texture);

        barcodeModel->shader(shader, true);

        return barcodeModel;
    }
}

ProductVisualSystem::ProductVisualSystem(AssetLoader& assetLoader)
    : m_assetLoader(assetLoader),
      m_barcodeTexture(Texture::LoadShared(assetLoader.assetPath(kBarcodeTexturePath).c_str()))
{
}

void ProductVisualSystem::createVisual(Product* product, std::list<BaseModel*>& models)
{
    if (product == nullptr || hasVisual(product))
    {
        return;
    }

    const ProductDefinition& definition = product->definition();

    Model* productModel = m_assetLoader.loadModel(definition.modelPath(), false);

    if (productModel == nullptr)
    {
        std::cerr << "Failed to load product visual: " << definition.modelPath() << "\n";

        return;
    }

    const AABB& boundingBox = productModel->boundingBox();

    Vector modelCenter = (boundingBox.Min + boundingBox.Max) * 0.5f;

    productModel->shader(new PhongShader(), true);

    ProductVisual visual;

    visual.product = product;

    visual.model = productModel;

    // Das Asset wird um seinen lokalen Mittelpunkt verschoben, damit der Product-Transform auf dem physikalischen Mittelpunkt statt auf dem OBJ-Ursprung liegt
    visual.localOffset = -modelCenter;

    visual.visualScale = definition.modelScale();

    visual.barcodeModel = createBarcodeModel(definition, m_barcodeTexture);

    // Der minimale Abstand zur Oberfläche verhindert Z-Fighting zwischen Produkt und Barcode
    visual.barcodeLocalPosition =
        definition.barcodePosition() + definition.barcodeNormal() * kBarcodeSurfaceOffset;

    updateVisualTransform(visual);

    models.push_back(productModel);

    if (visual.barcodeModel != nullptr)
    {
        models.push_back(visual.barcodeModel);
    }

    m_visuals.push_back(visual);
}

void ProductVisualSystem::update()
{
    for (ProductVisual& visual : m_visuals)
    {
        updateVisualTransform(visual);
    }
}

BaseModel* ProductVisualSystem::modelFor(const Product* product) const
{
    if (product == nullptr)
    {
        return nullptr;
    }

    for (const ProductVisual& visual : m_visuals)
    {
        if (visual.product == product)
        {
            return visual.model;
        }
    }

    return nullptr;
}

void ProductVisualSystem::removeVisuals(const std::unordered_set<Product*>& products,
                                        std::list<BaseModel*>& models)
{
    auto visualIterator = m_visuals.begin();

    while (visualIterator != m_visuals.end())
    {
        ProductVisual& visual = *visualIterator;

        bool shouldRemove =
            visual.product != nullptr && products.find(visual.product) != products.end();

        if (!shouldRemove)
        {
            ++visualIterator;

            continue;
        }

        removeVisual(visual, models);

        visualIterator = m_visuals.erase(visualIterator);
    }
}

void ProductVisualSystem::clear(std::list<BaseModel*>& models)
{
    for (ProductVisual& visual : m_visuals)
    {
        removeVisual(visual, models);
    }

    m_visuals.clear();
}

bool ProductVisualSystem::hasVisual(const Product* product) const
{
    if (product == nullptr)
    {
        return false;
    }

    for (const ProductVisual& visual : m_visuals)
    {
        if (visual.product == product)
        {
            return true;
        }
    }

    return false;
}

// Produktmodell und Barcode folgen demselben Gameplay-Transform, behalten aber eigene lokale Offsets
void ProductVisualSystem::updateVisualTransform(ProductVisual& visual) const
{
    if (visual.product == nullptr || visual.model == nullptr)
    {
        return;
    }

    Matrix scaleTransform;

    scaleTransform.scale(visual.visualScale, visual.visualScale, visual.visualScale);

    Matrix localOffsetTransform;

    localOffsetTransform.translation(visual.localOffset);

    visual.model->transform(visual.product->getWorldTransform() * scaleTransform
                            * localOffsetTransform);

    if (visual.barcodeModel != nullptr)
    {
        Matrix barcodeOffsetTransform;

        barcodeOffsetTransform.translation(visual.barcodeLocalPosition);

        visual.barcodeModel->transform(visual.product->getWorldTransform()
                                       * barcodeOffsetTransform);
    }
}

void ProductVisualSystem::removeVisual(ProductVisual& visual, std::list<BaseModel*>& models)
{
    if (visual.model != nullptr)
    {
        models.remove(visual.model);

        delete visual.model;

        visual.model = nullptr;
    }

    if (visual.barcodeModel != nullptr)
    {
        models.remove(visual.barcodeModel);

        delete visual.barcodeModel;

        visual.barcodeModel = nullptr;
    }

    visual.product = nullptr;
}
