#include "ShelfSystem.h"

#include "../../assets/AssetLoader.h"

#include "../../engine/Model.h"
#include "../../engine/PhongShader.h"
#include "../../engine/TriangleBoxModel.h"

#include "../scene/StoreLayout.h"

#include <array>
#include <cmath>
#include <utility>

namespace
{
    constexpr float kPi = 3.14159265358979323846f;

    constexpr float kMinimumModelDimension = 0.000001f;

    constexpr float kSpawnDistance = 0.08f;

    constexpr int kSlotCount = 3;

    struct ShelfAssetDefinition
    {
        const char* modelPath;

        Vector targetSize;

        std::array<float, 4> surfaceBackLocalYs;

        std::array<float, 4> surfaceFrontLocalYs;

        std::array<float, 4> boardBottomLocalYs;

        float usableMinLocalX;
        float usableMaxLocalX;

        float usableBackLocalZ;
        float usableFrontLocalZ;

        float backWallMinLocalZ;
        float backWallMaxLocalZ;
    };

    const ShelfAssetDefinition kShelfAsset = {
                                              "local/models/environment/shelf/shelf.obj",

                                              Vector(1.35f, 2.10f, 0.85f),

                                              {0.11752132f, 0.68066517f, 1.24380904f, 1.80695289f},

                                              {0.06468344f, 0.62782729f, 1.19097114f, 1.75411505f},

                                              {0.00922441f, 0.57236826f, 1.13551211f, 1.69865596f},

                                              -1.36194158f,
                                              1.36628652f,

                                              -0.34351201f,
                                              0.42097624f,

                                              -0.44733437f,
                                              -0.35049535f};

    struct BoxPart
    {
        Vector localCenter;

        Vector size;

        bool blocksInteraction;
    };

    struct ShelfSetup
    {
        Vector center;

        float frontDirectionZ;

        std::array<ProductType, 4> productTypes;
    };

    PhongShader* createShelfShader(const Color& color)
    {
        PhongShader* shader = new PhongShader();

        shader->diffuseColor(color);

        shader->ambientColor(color * 0.25f);

        shader->specularColor(Color(0.08f, 0.08f, 0.08f));

        shader->specularExp(10.0f);

        return shader;
    }

    TriangleBoxModel* createShelfPartModel(const Vector& center,
                                           const Vector& size,
                                           const Color& color)
    {
        TriangleBoxModel* model = new TriangleBoxModel(size.X, size.Y, size.Z);

        Matrix transform;

        transform.translation(center);

        model->transform(transform);

        model->shader(createShelfShader(color), true);

        return model;
    }
}

ShelfSystem::ShelfSystem(const AssetLoader& assetLoader,
                         const ProductCatalog& productCatalog,
                         ProductCollection& products,
                         ProductVisualSystem& productVisualSystem)
    : m_assetLoader(assetLoader),
      m_productCatalog(productCatalog),
      m_products(products),
      m_productVisualSystem(productVisualSystem),
      m_shelfModelBounds(),
      m_shelfModelScale(1.0f, 1.0f, 1.0f),
      m_shelfAssetReady(false),
      m_shelves(),
      m_refillCompletedEvent(false)
{
    m_shelfAssetReady = initializeShelfAsset();
}

bool ShelfSystem::initializeShelfAsset()
{
    bool modelAvailable = m_assetLoader.loadModelBounds(kShelfAsset.modelPath, m_shelfModelBounds);

    if (!modelAvailable)
    {
        m_shelfModelBounds = AABB(Vector(-1.46001339f, 0.0f, -0.44733437f),
                                  Vector(1.46001339f, 2.16651613f, 0.44733437f));
    }

    Vector modelSize = m_shelfModelBounds.size();

    if (modelSize.X <= kMinimumModelDimension || modelSize.Y <= kMinimumModelDimension
        || modelSize.Z <= kMinimumModelDimension)
    {
        return false;
    }

    m_shelfModelScale = Vector(kShelfAsset.targetSize.X / modelSize.X,
                               kShelfAsset.targetSize.Y / modelSize.Y,
                               kShelfAsset.targetSize.Z / modelSize.Z);

    return modelAvailable;
}

Vector ShelfSystem::shelfModelCenter() const
{
    return m_shelfModelBounds.center();
}

float ShelfSystem::centeredScaledY(float localY) const
{
    return (localY - shelfModelCenter().Y) * m_shelfModelScale.Y;
}

float ShelfSystem::centeredScaledZ(float localZ) const
{
    return (localZ - shelfModelCenter().Z) * m_shelfModelScale.Z;
}

float ShelfSystem::shelfSurfaceLocalY(std::size_t surfaceIndex, float localZ) const
{
    if (surfaceIndex >= kShelfAsset.surfaceBackLocalYs.size())
    {
        return 0.0f;
    }

    if (!m_shelfAssetReady)
    {
        return kShelfAsset.surfaceBackLocalYs[surfaceIndex];
    }

    float depthRange = kShelfAsset.usableFrontLocalZ - kShelfAsset.usableBackLocalZ;

    if (depthRange <= kMinimumModelDimension)
    {
        return kShelfAsset.surfaceBackLocalYs[surfaceIndex];
    }

    float depthFactor = (localZ - kShelfAsset.usableBackLocalZ) / depthRange;

    if (depthFactor < 0.0f)
    {
        depthFactor = 0.0f;
    }
    else if (depthFactor > 1.0f)
    {
        depthFactor = 1.0f;
    }

    float backY = kShelfAsset.surfaceBackLocalYs[surfaceIndex];
    float frontY = kShelfAsset.surfaceFrontLocalYs[surfaceIndex];

    return backY + (frontY - backY) * depthFactor;
}

float ShelfSystem::shelfUsableWidth() const
{
    return (kShelfAsset.usableMaxLocalX - kShelfAsset.usableMinLocalX) * m_shelfModelScale.X;
}

float ShelfSystem::shelfUsableDepth() const
{
    return (kShelfAsset.usableFrontLocalZ - kShelfAsset.usableBackLocalZ) * m_shelfModelScale.Z;
}

float ShelfSystem::shelfBackWallDepth() const
{
    return (kShelfAsset.backWallMaxLocalZ - kShelfAsset.backWallMinLocalZ) * m_shelfModelScale.Z;
}

void ShelfSystem::create(std::list<BaseModel*>& models,
                         std::list<AABB>& playerColliders,
                         std::list<AABB>& interactionColliders,
                         std::list<AABB>& physicsColliders)
{
    m_shelves.clear();
    
    m_refillCompletedEvent = false;

    const std::array<Vector, 2>& shelfCenters = StoreLayout::shelfCenters();

    const std::array<float, 2>& frontDirections = StoreLayout::shelfFrontDirections();

    const std::array<ShelfSetup, 2> shelfSetups = {ShelfSetup{shelfCenters[0],
                                                              frontDirections[0],
                                                              {ProductType::Cereal,
                                                               ProductType::SodaCan,
                                                               ProductType::Detergent,
                                                               ProductType::Ketchup}},

                                                   ShelfSetup{shelfCenters[1],
                                                              frontDirections[1],
                                                              {ProductType::Mustard,
                                                               ProductType::PotatoChips,
                                                               ProductType::Butter,
                                                               ProductType::Tomato}}};

    for (const ShelfSetup& setup : shelfSetups)
    {
        createOpenShelf(setup.center,
                        setup.frontDirectionZ,
                        models,
                        playerColliders,
                        interactionColliders,
                        physicsColliders);

        createProductRows(setup.center, setup.frontDirectionZ, setup.productTypes, models);
    }
}

void ShelfSystem::update(float deltaTime,
                         std::list<BaseModel*>& models,
                         std::list<IInteractable*>& interactables,
                         std::vector<IInteractable*>& pendingInteractables)
{
    for (Shelf& shelf : m_shelves)
    {
        shelf.update(deltaTime);

        for (ShelfProductRow& row : shelf.rows())
        {
            if (row.consumeRefillCompletedEvent())
            {
                m_refillCompletedEvent = true;
            }
        }
    }

    refillRows(models, interactables, pendingInteractables);
}

bool ShelfSystem::consumeRefillCompletedEvent()
{
    bool refillCompleted = m_refillCompletedEvent;

    m_refillCompletedEvent = false;

    return refillCompleted;
}

BaseModel* ShelfSystem::createShelfVisual(const Vector& center, float frontDirectionZ) const
{
    if (!m_shelfAssetReady)
    {
        return nullptr;
    }

    Model* shelfModel = m_assetLoader.loadModel(kShelfAsset.modelPath, false);

    if (shelfModel == nullptr)
    {
        return nullptr;
    }

    Matrix centerOffsetTransform;
    centerOffsetTransform.translation(-shelfModelCenter());

    Matrix scaleTransform;
    scaleTransform.scale(m_shelfModelScale);

    Matrix rotationTransform;

    if (frontDirectionZ < 0.0f)
    {
        rotationTransform.rotationY(kPi);
    }
    else
    {
        rotationTransform.identity();
    }

    Matrix translationTransform;
    translationTransform.translation(center);

    shelfModel->transform(translationTransform * rotationTransform * scaleTransform
                          * centerOffsetTransform);
    shelfModel->shader(new PhongShader(), true);

    return shelfModel;
}

void ShelfSystem::createOpenShelf(const Vector& center,
                                  float frontDirectionZ,
                                  std::list<BaseModel*>& models,
                                  std::list<AABB>& playerColliders,
                                  std::list<AABB>& interactionColliders,
                                  std::list<AABB>& physicsColliders)
{
    BaseModel* shelfVisual = createShelfVisual(center, frontDirectionZ);

    if (shelfVisual != nullptr)
    {
        models.push_back(shelfVisual);
    }

    float usableWidth = shelfUsableWidth();

    float usableDepth = shelfUsableDepth();

    float usableCenterLocalZ =
        (kShelfAsset.usableBackLocalZ + kShelfAsset.usableFrontLocalZ) * 0.5f;

    float usableCenterOffsetZ = frontDirectionZ * centeredScaledZ(usableCenterLocalZ);

    float backWallCenterLocalZ =
        (kShelfAsset.backWallMinLocalZ + kShelfAsset.backWallMaxLocalZ) * 0.5f;

    float backWallCenterOffsetZ = frontDirectionZ * centeredScaledZ(backWallCenterLocalZ);

    // Die Collider werden aus vermessenen Asset-Bereichen aufgebaut, statt das gesamte Regal als eine massive Box zu behandeln
    std::array<BoxPart, 7> shelfParts;

    for (std::size_t surfaceIndex = 0; surfaceIndex < 4; ++surfaceIndex)
    {
        float topLocalY = kShelfAsset.surfaceBackLocalYs[surfaceIndex];

        float bottomLocalY = kShelfAsset.boardBottomLocalYs[surfaceIndex];

        float boardHeight = (topLocalY - bottomLocalY) * m_shelfModelScale.Y;

        float boardCenterLocalY = (topLocalY + bottomLocalY) * 0.5f;

        shelfParts[surfaceIndex] =
            BoxPart{Vector(0.0f, centeredScaledY(boardCenterLocalY), usableCenterOffsetZ),
                    Vector(usableWidth, boardHeight, usableDepth),
                    false};
    }

    shelfParts[4] =
        BoxPart{Vector(0.0f, 0.0f, backWallCenterOffsetZ),
                Vector(kShelfAsset.targetSize.X, kShelfAsset.targetSize.Y, shelfBackWallDepth()),
                true};

    float sideWallWidth = kShelfAsset.targetSize.X - usableWidth;

    sideWallWidth *= 0.5f;

    float sideWallOffsetX = kShelfAsset.targetSize.X * 0.5f - sideWallWidth * 0.5f;

    shelfParts[5] =
        BoxPart{Vector(-sideWallOffsetX, 0.0f, 0.0f),
                Vector(sideWallWidth, kShelfAsset.targetSize.Y, kShelfAsset.targetSize.Z),
                true};

    shelfParts[6] =
        BoxPart{Vector(sideWallOffsetX, 0.0f, 0.0f),
                Vector(sideWallWidth, kShelfAsset.targetSize.Y, kShelfAsset.targetSize.Z),
                true};

    for (const BoxPart& part : shelfParts)
    {
        Vector worldCenter = center + part.localCenter;

        if (shelfVisual == nullptr)
        {
            models.push_back(
                createShelfPartModel(worldCenter, part.size, Color(0.45f, 0.25f, 0.12f)));
        }

        AABB partCollider = AABB::fromCenterAndSize(worldCenter, part.size);

        if (part.blocksInteraction)
        {
            interactionColliders.push_back(partCollider);
        }

        physicsColliders.push_back(partCollider);
    }

    playerColliders.push_back(AABB::fromCenterAndSize(center, kShelfAsset.targetSize));
}

std::vector<Vector> ShelfSystem::createSlotPositions(const Vector& shelfCenter,
                                                     float frontDirectionZ,
                                                     std::size_t surfaceIndex,
                                                     const Vector& productSize) const
{
    float frontOffset = centeredScaledZ(kShelfAsset.usableFrontLocalZ) - productSize.Z * 0.5f;

    float backOffset = centeredScaledZ(kShelfAsset.usableBackLocalZ) + productSize.Z * 0.5f;

    float slotDistance = 0.0f;

    if (kSlotCount > 1)
    {
        slotDistance = (frontOffset - backOffset) / static_cast<float>(kSlotCount - 1);
    }

    if (slotDistance < 0.0f)
    {
        slotDistance = 0.0f;
    }

    std::vector<Vector> slotPositions;

    slotPositions.reserve(kSlotCount);

    // Slot 0 liegt vorne, weitere Slots laufen gleichmäßig bis an die Rückseite des nutzbaren Regalbereichs
    for (int slotIndex = 0; slotIndex < kSlotCount; ++slotIndex)
    {
        float depthOffset = frontOffset - slotDistance * static_cast<float>(slotIndex);

        float originalLocalZ = shelfModelCenter().Z + depthOffset / m_shelfModelScale.Z;

        float surfaceLocalY = shelfSurfaceLocalY(surfaceIndex, originalLocalZ);

        float productCenterY =
            shelfCenter.Y + centeredScaledY(surfaceLocalY) + productSize.Y * 0.5f;

        slotPositions.emplace_back(shelfCenter.X,
                                   productCenterY,
                                   shelfCenter.Z + frontDirectionZ * depthOffset);
    }

    return slotPositions;
}

Vector ShelfSystem::createSpawnPosition(const std::vector<Vector>& slotPositions) const
{
    if (slotPositions.empty())
    {
        return Vector();
    }

    if (slotPositions.size() < 2)
    {
        return slotPositions.back();
    }

    // Nachfüllprodukte entstehen knapp hinter dem letzten Slot und fahren anschließend sichtbar ins Regal
    Vector spawnDirection = slotPositions.back() - slotPositions[slotPositions.size() - 2];

    if (spawnDirection.lengthSquared() <= 0.000001f)
    {
        return slotPositions.back();
    }

    spawnDirection.normalize();

    return slotPositions.back() + spawnDirection * kSpawnDistance;
}

// Erzeugt pro Regalflaeche eine Reihe mit festen Slots und verbindet sie mit genau einem Produkttyp
void ShelfSystem::createProductRows(const Vector& shelfCenter,
                                    float frontDirectionZ,
                                    const std::array<ProductType, 4>& productTypes,
                                    std::list<BaseModel*>& models)
{
    const std::array<std::size_t, 4> surfaceIndices = {3, 2, 1, 0};

    Shelf shelf;

    for (std::size_t rowIndex = 0; rowIndex < productTypes.size(); ++rowIndex)
    {
        ProductType productType = productTypes[rowIndex];

        const ProductDefinition* definition = m_productCatalog.find(productType);

        if (definition == nullptr)
        {
            continue;
        }

        std::vector<Vector> slotPositions = createSlotPositions(shelfCenter,
                                                                frontDirectionZ,
                                                                surfaceIndices[rowIndex],
                                                                definition->boundsSize());

        shelf.addRow(productType,
                     slotPositions,
                     createSpawnPosition(slotPositions),
                     frontDirectionZ);
    }

    for (ShelfProductRow& row : shelf.rows())
    {
        createInitialProducts(row, models);
    }

    shelf.update(0.0f);

    m_shelves.push_back(std::move(shelf));
}

void ShelfSystem::createInitialProducts(ShelfProductRow& row, std::list<BaseModel*>& models)
{
    for (std::size_t slotIndex = 0; slotIndex < row.slots().size(); ++slotIndex)
    {
        Product* product = m_products.create(row.productType());

        if (product == nullptr)
        {
            continue;
        }

        if (!row.addProduct(product))
        {
            m_products.remove(product);

            continue;
        }

        m_productVisualSystem.createVisual(product, models);
    }
}

void ShelfSystem::refillRows(std::list<BaseModel*>& models,
                             std::list<IInteractable*>& interactables,
                             std::vector<IInteractable*>& pendingInteractables)
{
    for (Shelf& shelf : m_shelves)
    {
        for (ShelfProductRow& row : shelf.rows())
        {
            if (!row.needsRefillProduct())
            {
                continue;
            }

            Product* product = m_products.create(row.productType());

            if (product == nullptr)
            {
                continue;
            }

            if (!row.addRefillProduct(product))
            {
                m_products.remove(product);

                continue;
            }

            m_productVisualSystem.createVisual(product, models);

            interactables.push_back(product);

            // InteractionSystem wird außerhalb des Scene-Updates aktualisiert, deshalb wird das neue Produkt zusätzlich vorgemerkt
            pendingInteractables.push_back(product);
        }
    }
}
