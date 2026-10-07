#include "StoreScene.h"
#include "../../engine/Model.h"
#include "StoreLayout.h"
#include "../../engine/PhongShader.h"

#include "../products/Product.h"
#include "../rendering/HighlightRenderer.h"

#include <cmath>
#include <unordered_set>

namespace
{
    // Kleine, rein visuelle Rotation fuer eine erkennbare Wischbewegung
    constexpr float kWipeAnimationSpeed = 8.0f;
    constexpr float kWipeAnimationAngle = 0.12f;
}

StoreScene::StoreScene(AssetLoader& assetLoader)
    : m_productCatalog(assetLoader),
      m_products(m_productCatalog),
      m_productVisualSystem(assetLoader),
      m_environmentBuilder(assetLoader),
      m_scannerStation(assetLoader),
      m_shelfSystem(assetLoader, m_productCatalog, m_products, m_productVisualSystem),
      m_deliveryBoxScene(assetLoader),
      m_cleaningToolModel(nullptr),
      m_cleaningTool(),
      m_cleaningAnimationTime(0.0f),
      m_cleaningAnimationActive(false),
      m_spill(),
      m_cleaningSystem(),
      m_spillModel(nullptr)
{
}

void StoreScene::create(std::list<BaseModel*>& models)
{
    m_staticColliders.clear();
    m_interactionColliders.clear();
    m_physicsColliders.clear();

    m_interactables.clear();
    m_pendingInteractables.clear();

    m_productVisualSystem.clear(models);

    m_cleaningAnimationTime = 0.0f;
    m_cleaningAnimationActive = false;

    m_products.clear();

    Product::resetIdCounter();

    // Hier werden Modelle, Collider und Interaktionsobjekte der Teilsysteme zu einer gemeinsamen Szene verbunden
    m_environmentBuilder.create(models,
                                m_staticColliders,
                                m_interactionColliders,
                                m_physicsColliders);

    Vector cleaningToolSize;

    m_cleaningToolModel =
        m_environmentBuilder.createCleaningToolVisual(m_cleaningToolVisualOffset, cleaningToolSize);

    if (m_cleaningToolModel != nullptr)
    {
        Vector cleaningToolCenter = StoreLayout::cleaningToolCenter();

        cleaningToolCenter.Y = cleaningToolSize.Y * 0.5f;

        m_cleaningTool.configure(cleaningToolCenter, cleaningToolSize);

        m_cleaningToolModel->transform(m_cleaningTool.getWorldTransform()
                                       * m_cleaningToolVisualOffset);

        models.push_back(m_cleaningToolModel);

        m_interactables.push_back(&m_cleaningTool);
    }

    m_spill.configure(StoreLayout::cleaningSpotSize(), 4.0f);

    const std::array<Vector, 2>& cleaningSpotCenters = StoreLayout::cleaningSpotCenters();

    std::vector<Vector> cleaningSpawnPoints(cleaningSpotCenters.begin(), cleaningSpotCenters.end());

    m_cleaningSystem.initialize(
        &m_spill,
        cleaningSpawnPoints
    );

    m_spillModel = m_environmentBuilder.createSpillVisual();

    if (m_spillModel != nullptr)
    {
        models.push_back(m_spillModel);

        updateSpillVisual();
    }

    m_shelfSystem.create(models, m_staticColliders, m_interactionColliders, m_physicsColliders);

    m_deliveryBoxScene.create(models, m_interactables, m_interactionColliders);

    m_scannerStation.create(models, m_staticColliders, m_physicsColliders);

    registerAllProducts();

    updatePlayerDynamicColliders();

    updateCarryBlockingColliders();
}

void StoreScene::update(float deltaTime, std::list<BaseModel*>& models)
{
    m_shelfSystem.update(deltaTime, models, m_interactables, m_pendingInteractables);

    m_deliveryBoxScene.update(deltaTime, m_products.products());

    m_productVisualSystem.update();

    if (m_cleaningToolModel != nullptr)
    {
        Matrix cleaningToolVisualTransform = m_cleaningTool.getWorldTransform();

        // Nur das sichtbare Modell wird bewegt
        // Gameplay-Transform, Collider und cleaningPosition() bleiben unveraendert, damit die Wischanimation keine Kollision oder Reinigungslogik beeinflusst
        
        if (m_cleaningAnimationActive)
        {
            Matrix wipeRotation;

            float wipeAngle =
                std::sin(m_cleaningAnimationTime * kWipeAnimationSpeed) * kWipeAnimationAngle;

            wipeRotation.rotationZ(wipeAngle);

            cleaningToolVisualTransform *= wipeRotation;
        }
        else
        {
            m_cleaningAnimationTime = 0.0f;
        }

        m_cleaningToolModel->transform(cleaningToolVisualTransform
                                       * m_cleaningToolVisualOffset);
    }

    // cleanSpill() setzt das Flag nur fuer Frames, in denen wirklich gereinigt wird
    m_cleaningAnimationActive = false;

    m_cleaningSystem.update(deltaTime);

    updateSpillVisual();

    updatePlayerDynamicColliders();

    updateCarryBlockingColliders();
}

const std::list<AABB>& StoreScene::staticColliders() const
{
    return m_staticColliders;
}

const std::list<AABB>& StoreScene::interactionColliders() const
{
    return m_interactionColliders;
}

const std::list<AABB>& StoreScene::physicsColliders() const
{
    return m_physicsColliders;
}

const std::list<IInteractable*>& StoreScene::interactables() const
{
    return m_interactables;
}

const std::vector<IInteractable*>& StoreScene::pendingInteractables() const
{
    return m_pendingInteractables;
}

void StoreScene::clearPendingInteractables()
{
    m_pendingInteractables.clear();
}

const ProductCatalog& StoreScene::productCatalog() const
{
    return m_productCatalog;
}

ScannerStation& StoreScene::scannerStation()
{
    return m_scannerStation;
}

const ScannerStation& StoreScene::scannerStation() const
{
    return m_scannerStation;
}

DeliveryBox& StoreScene::deliveryBox()
{
    return m_deliveryBoxScene.deliveryBox();
}

const DeliveryBox& StoreScene::deliveryBox() const
{
    return m_deliveryBoxScene.deliveryBox();
}

const DeliveryZone& StoreScene::deliveryZone() const
{
    return m_deliveryBoxScene.deliveryZone();
}

const std::vector<Product*>& StoreScene::products() const
{
    return m_products.products();
}

const Matrix& StoreScene::deliveryBoxBackLidPhysicsTransform() const
{
    return m_deliveryBoxScene.backLidPhysicsTransform();
}

const Matrix& StoreScene::deliveryBoxFrontLidPhysicsTransform() const
{
    return m_deliveryBoxScene.frontLidPhysicsTransform();
}

const Vector& StoreScene::deliveryBoxLidPhysicsSize() const
{
    return m_deliveryBoxScene.lidPhysicsSize();
}

const std::vector<AABB>& StoreScene::deliveryBoxCarryColliders() const
{
    return m_deliveryBoxScene.carryColliders();
}

const std::array<Matrix, 5>& StoreScene::deliveryBoxBodyPhysicsTransforms() const
{
    return m_deliveryBoxScene.bodyPhysicsTransforms();
}

const std::array<Vector, 5>& StoreScene::deliveryBoxBodyPhysicsSizes() const
{
    return m_deliveryBoxScene.bodyPhysicsSizes();
}

bool StoreScene::consumeDeliveryBoxBlockedEvent()
{
    return m_deliveryBoxScene.consumeBlockedEvent();
}

bool StoreScene::consumeDeliveryBoxSealedEvent()
{
    return m_deliveryBoxScene.consumeSealedEvent();
}

bool StoreScene::sealDeliveryBox(int orderId)
{
    return m_deliveryBoxScene.seal(orderId, m_products.products());
}

std::vector<Product*> StoreScene::sealedDeliveryBoxProducts() const
{
    return m_deliveryBoxScene.sealedProducts(m_products.products());
}

bool StoreScene::isDeliveryBoxInDeliveryZone() const
{
    return m_deliveryBoxScene.isInDeliveryZone();
}

bool StoreScene::isPlayerNearDeliveryZone(const Vector& playerPosition) const
{
    return m_deliveryBoxScene.isPlayerNearDeliveryZone(playerPosition);
}

void StoreScene::removeDeliveredProducts(const std::vector<Product*>& deliveredProducts,
                                         std::list<BaseModel*>& models)
{
    std::unordered_set<Product*> productsToRemove;

    for (Product* product : deliveredProducts)
    {
        if (product == nullptr)
        {
            continue;
        }

        product->setState(ProductState::Delivered);

        product->setInteractable(false);

        productsToRemove.insert(product);

        m_interactables.remove(product);
    }

    m_productVisualSystem.removeVisuals(productsToRemove, models);

    m_products.remove(productsToRemove);
}

void StoreScene::resetDeliveryBox()
{
    m_deliveryBoxScene.reset();
}

void StoreScene::registerAllProducts()
{
    for (Product* product : m_products.products())
    {
        if (product == nullptr)
        {
            continue;
        }

        m_interactables.push_back(product);
    }
}

const AABB& StoreScene::terminalBounds() const
{
    return m_environmentBuilder.terminalBounds();
}

void StoreScene::updateSpillVisual()
{
    if (m_spillModel == nullptr)
    {
        return;
    }
    
    if (!m_spill.isActive())
    {
        Matrix hiddenTransform;

        hiddenTransform.translation(
            Vector(0.0f, -100.0f, 0.0f)
        );

        m_spillModel->transform(hiddenTransform);

        return;
    }

    Vector size = m_spill.currentSize();

    Matrix scaleTransform;

    scaleTransform.scale(Vector(size.X, 1.0f, size.Z));

    Matrix translationTransform;

    Vector position = m_spill.position();

    position.Y += 0.002f;

    translationTransform.translation(position);

    m_spillModel->transform(translationTransform * scaleTransform);

    PhongShader* spillShader = dynamic_cast<PhongShader*>(m_spillModel->shader());

    if (spillShader != nullptr)
    {
        // Der Spill wird proportional zum Reinigungsfortschritt transparenter
        spillShader->alphaMultiplier(1.0f - m_spill.cleaningProgress());
    }
}

bool StoreScene::canCleanSpill() const
{
    return m_cleaningTool.isCarried()
           && m_spill.isActive()
           && m_spill.contains(
               m_cleaningTool.cleaningPosition()
           );
}

bool StoreScene::cleanSpill(float deltaTime)
{
    if (!canCleanSpill())
    {
        return false;
    }

    m_cleaningAnimationActive = true;
    m_cleaningAnimationTime += deltaTime;

    return m_spill.addCleaningProgress(deltaTime);
}

bool StoreScene::isSpillActive() const
{
    return m_cleaningSystem.isSpillActive();
}

bool StoreScene::consumeShelfRefillCompletedEvent()
{
    return m_shelfSystem.consumeRefillCompletedEvent();
}

bool StoreScene::consumeSpillStartedEvent()
{
    return m_cleaningSystem.consumeSpillStartedEvent();
}

void StoreScene::registerHighlightModels(HighlightRenderer& highlighter) const
{
    // Wischmop
    if (m_cleaningToolModel != nullptr)
    {
        highlighter.registerModel(&m_cleaningTool, m_cleaningToolModel);
    }

    // Produkte
    for (Product* product : m_products.products())
    {
        BaseModel* model = m_productVisualSystem.modelFor(product);

        if (model != nullptr)
        {
            highlighter.registerModel(product, model);
        }
    }

    IInteractable* deliveryBoxInteraction = m_deliveryBoxScene.interaction();

    if (deliveryBoxInteraction != nullptr)
    {
        for (BaseModel* model : m_deliveryBoxScene.highlightModels())
        {
            highlighter.registerModel(deliveryBoxInteraction, model);
        }
    }
}

BaseModel* StoreScene::terminalVisual() const
{
    return m_environmentBuilder.terminalVisual();
}

void StoreScene::updatePlayerDynamicColliders()
{
    // Bewegliche Hindernisse werden pro Frame neu aufgebaut, weil Karton und Mop ihren Standort aendern koennen
    m_playerDynamicColliders = m_deliveryBoxScene.carryColliders();

    if (m_cleaningToolModel != nullptr && !m_cleaningTool.isCarried())
    {
        m_playerDynamicColliders.push_back(m_cleaningTool.getCollider());
    }
}

const std::vector<AABB>& StoreScene::playerDynamicColliders() const
{
    return m_playerDynamicColliders;
}

bool StoreScene::hasCleaningTool() const
{
    return m_cleaningToolModel != nullptr;
}

Matrix StoreScene::cleaningToolTransform() const
{
    return m_cleaningTool.getWorldTransform();
}

Vector StoreScene::cleaningToolPhysicsSize() const
{
    return m_cleaningTool.getLocalBounds().size();
}

void StoreScene::updateCarryBlockingColliders()
{
    // Dieselben beweglichen Hindernisse begrenzen auch die erlaubte Position getragener Objekte
    m_carryBlockingColliders = m_deliveryBoxScene.carryColliders();

    if (m_cleaningToolModel != nullptr && !m_cleaningTool.isCarried())
    {
        m_carryBlockingColliders.push_back(m_cleaningTool.getCollider());
    }
}

const std::vector<AABB>& StoreScene::carryBlockingColliders() const
{
    return m_carryBlockingColliders;
}
