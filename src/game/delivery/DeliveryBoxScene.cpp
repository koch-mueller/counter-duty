#include "DeliveryBoxScene.h"

#include "../../assets/AssetLoader.h"
#include "../scene/StoreLayout.h"

#include "../../engine/Model.h"
#include "../../engine/PhongShader.h"

#include "DeliveryBoxGeometry.h"
#include "DeliveryBoxInteraction.h"

#include "../collision/ConvexBoxCollision.h"
#include "../interaction/IInteractable.h"
#include "../products/Product.h"

#include <cstddef>
#include <stdexcept>

namespace
{
    constexpr float kPi = 3.14159265358979323846f;

    constexpr std::size_t kBottomPartIndex = 0;

    constexpr float kMinimumBottomPhysicsThickness = 0.08f;

    PhongShader* createColoredShader(const Color& diffuse)
    {
        PhongShader* shader = new PhongShader();

        shader->diffuseColor(diffuse);

        shader->ambientColor(diffuse * 0.25f);

        shader->specularColor(Color(0.08f, 0.08f, 0.08f));

        shader->specularExp(10.0f);

        return shader;
    }
}

DeliveryBoxScene::DeliveryBoxScene(AssetLoader& assetLoader)
    : m_assetLoader(assetLoader),
      m_deliveryBox(),
      m_deliveryZone(StoreLayout::deliveryZoneBounds(),
                     StoreLayout::deliveryZoneInteractionDistance()),
      m_resetPosition(StoreLayout::deliveryBoxResetPosition()),
      m_interaction(nullptr),
      m_bodyModel(nullptr),
      m_backLidModel(nullptr),
      m_frontLidModel(nullptr),
      m_openLidAngle(0.25f - kPi * 0.5f),
      m_lidAngle(0.25f - kPi * 0.5f),
      m_targetLidAngle(5.15f - kPi),
      m_lidSpeed(2.5f),
      m_blockedEvent(false),
      m_sealedEvent(false)
{
}

DeliveryBoxScene::~DeliveryBoxScene() = default;

void DeliveryBoxScene::create(std::list<BaseModel*>& models,
                              std::list<IInteractable*>& interactables,
                              std::list<AABB>& interactionColliders)
{
    m_blockedEvent = false;
    m_sealedEvent = false;

    m_carryColliders.clear();

    m_bodyModel = m_assetLoader.loadModel("models/delivery_box/"
                                          "delivery_box_body.obj",
                                          false);

    m_backLidModel = m_assetLoader.loadModel("models/delivery_box/"
                                             "delivery_box_lid.obj",
                                             false);

    m_frontLidModel = m_assetLoader.loadModel("models/delivery_box/"
                                              "delivery_box_lid.obj",
                                              false);

    bool modelsLoaded =
        m_bodyModel != nullptr && m_backLidModel != nullptr && m_frontLidModel != nullptr;

    if (!modelsLoaded)
    {
        delete m_bodyModel;
        delete m_backLidModel;
        delete m_frontLidModel;

        m_bodyModel = nullptr;
        m_backLidModel = nullptr;
        m_frontLidModel = nullptr;

        throw std::runtime_error("Delivery box models could not be loaded");
    }

    const Color cardboardColor(0.8f, 0.7f, 0.5f);

    m_bodyModel->shader(createColoredShader(cardboardColor), true);

    m_backLidModel->shader(createColoredShader(cardboardColor), true);

    m_frontLidModel->shader(createColoredShader(cardboardColor), true);

    DeliveryBoxGeometrySettings settings;

    settings.wallThicknessRatio = 0.025f;

    settings.bottomThicknessRatio = 0.08f;

    settings.lidClearanceHeightRatio = 0.25f;

    // Die Kollisionsgeometrie wird aus den tatsächlichen Asset-Bounds abgeleitet und bleibt damit unabhängig vom Modellmaßstab
    DeliveryBoxGeometry geometry = createDeliveryBoxGeometry(m_bodyModel->boundingBox(),
                                                             m_backLidModel->boundingBox(),
                                                             m_backLidModel->localVertices(),
                                                             settings);

    const float deliveryBoxScale = 3.6f;

    m_deliveryBox.setGeometry(geometry, m_resetPosition, deliveryBoxScale);

    m_lidAngle = m_openLidAngle;

    m_interaction = std::make_unique<DeliveryBoxInteraction>(&m_deliveryBox);

    updateTransforms();

    interactables.push_back(m_interaction.get());

    models.push_back(m_bodyModel);

    models.push_back(m_backLidModel);

    models.push_back(m_frontLidModel);

    for (const DeliveryBox::DeliveryBoxPart& part : m_deliveryBox.parts())
    {
        interactionColliders.push_back(AABB::fromCenterAndSize(part.center, part.size));
    }
}

void DeliveryBoxScene::update(float deltaTime, const std::vector<Product*>& products)
{
    updateLidAnimation(deltaTime, products);

    // Auch ein versiegelter Karton kann durch die Physik verschoben werden
    updateTransforms();

    m_deliveryBox.updateSealedContents(products);
}

DeliveryBox& DeliveryBoxScene::deliveryBox()
{
    return m_deliveryBox;
}

const DeliveryBox& DeliveryBoxScene::deliveryBox() const
{
    return m_deliveryBox;
}

const DeliveryZone& DeliveryBoxScene::deliveryZone() const
{
    return m_deliveryZone;
}

const Matrix& DeliveryBoxScene::backLidPhysicsTransform() const
{
    return m_backLidPhysicsTransform;
}

const Matrix& DeliveryBoxScene::frontLidPhysicsTransform() const
{
    return m_frontLidPhysicsTransform;
}

const Vector& DeliveryBoxScene::lidPhysicsSize() const
{
    return m_lidPhysicsSize;
}

const std::vector<AABB>& DeliveryBoxScene::carryColliders() const
{
    return m_carryColliders;
}

const std::array<Matrix, 5>& DeliveryBoxScene::bodyPhysicsTransforms() const
{
    return m_bodyPhysicsTransforms;
}

const std::array<Vector, 5>& DeliveryBoxScene::bodyPhysicsSizes() const
{
    return m_bodyPhysicsSizes;
}

bool DeliveryBoxScene::consumeBlockedEvent()
{
    if (!m_blockedEvent)
    {
        return false;
    }

    m_blockedEvent = false;

    return true;
}

bool DeliveryBoxScene::consumeSealedEvent()
{
    if (!m_sealedEvent)
    {
        return false;
    }

    m_sealedEvent = false;

    return true;
}

bool DeliveryBoxScene::seal(int orderId, const std::vector<Product*>& products)
{
    return m_deliveryBox.sealContents(orderId, products);
}

std::vector<Product*> DeliveryBoxScene::sealedProducts(const std::vector<Product*>& products) const
{
    return m_deliveryBox.sealedProducts(products);
}

bool DeliveryBoxScene::isInDeliveryZone() const
{
    return m_deliveryZone.containsFully(m_deliveryBox);
}

bool DeliveryBoxScene::isPlayerNearDeliveryZone(const Vector& playerPosition) const
{
    return m_deliveryZone.isPlayerNearby(playerPosition);
}

void DeliveryBoxScene::reset()
{
    m_deliveryBox.resetForNextOrder(m_resetPosition);

    m_lidAngle = m_openLidAngle;

    m_blockedEvent = false;
    m_sealedEvent = false;

    updateTransforms();
}

void DeliveryBoxScene::updateLidAnimation(float deltaTime, const std::vector<Product*>& products)
{
    DeliveryBoxState state = m_deliveryBox.state();

    if (state == DeliveryBoxState::Opening || state == DeliveryBoxState::Blocked)
    {
        m_lidAngle -= m_lidSpeed * deltaTime;

        if (m_lidAngle <= m_openLidAngle)
        {
            m_lidAngle = m_openLidAngle;

            updateTransforms();

            m_deliveryBox.finishOpening();

            return;
        }

        updateTransforms();

        return;
    }

    if (state != DeliveryBoxState::Closing)
    {
        return;
    }

    // Der vorherige Winkel wird benötigt, damit ein kollidierender Deckelschritt vollständig zurückgenommen werden kann
    float previousLidAngle = m_lidAngle;

    m_lidAngle += m_lidSpeed * deltaTime;

    if (m_lidAngle > m_targetLidAngle)
    {
        m_lidAngle = m_targetLidAngle;
    }

    updateTransforms();

    if (isLidBlocked(products))
    {
        m_lidAngle = previousLidAngle;

        updateTransforms();

        m_deliveryBox.blockClosing();

        m_blockedEvent = true;

        return;
    }

    if (m_lidAngle >= m_targetLidAngle)
    {
        m_deliveryBox.finishClosing();

        m_sealedEvent = true;
    }
}

void DeliveryBoxScene::updateTransforms()
{
    updateBodyPhysicsTransforms();

    if (m_bodyModel == nullptr || m_backLidModel == nullptr || m_frontLidModel == nullptr)
    {
        return;
    }

    const DeliveryBoxGeometry& geometry = m_deliveryBox.geometry();

    const Matrix& boxWorldTransform = m_deliveryBox.worldTransform();

    float boxWorldScale = m_deliveryBox.worldScale();

    Matrix scaleTransform;

    scaleTransform.scale(boxWorldScale);

    Matrix boxModelTransform = boxWorldTransform * scaleTransform;

    m_bodyModel->transform(boxModelTransform);

    // Rotation um das Scharnier: erst zum Pivot verschieben, rotieren und anschließend den Pivot wieder zurücknehmen
    Matrix hingeTranslation;

    hingeTranslation.translation(geometry.lidHingePosition);

    Matrix inverseHingeTranslation;

    inverseHingeTranslation.translation(-geometry.lidHingePosition);

    Matrix lidRotation;

    lidRotation.rotationX(m_lidAngle);

    m_backLidTransform =
        boxModelTransform * hingeTranslation * lidRotation * inverseHingeTranslation;

    m_backLidModel->transform(m_backLidTransform);

    Vector frontHingePosition(geometry.lidHingePosition.X,
                              geometry.lidHingePosition.Y,
                              geometry.outerBounds.Max.Z);

    Matrix frontHingeTranslation;

    frontHingeTranslation.translation(frontHingePosition);

    Matrix frontDirectionRotation;

    frontDirectionRotation.rotationY(kPi);

    m_frontLidTransform = boxModelTransform * frontHingeTranslation * frontDirectionRotation
                          * lidRotation * inverseHingeTranslation;

    m_frontLidModel->transform(m_frontLidTransform);

    m_backLidPhysicsTransform = createLidPhysicsTransform(m_backLidTransform);

    m_frontLidPhysicsTransform = createLidPhysicsTransform(m_frontLidTransform);

    m_lidPhysicsSize = geometry.lidPhysicsCollider.size * boxWorldScale;

    updateLidColliderVertices();

    // Während der Karton selbst getragen wird, darf er nicht als eigener Carry-Blocker auftauchen
    m_carryColliders.clear();

    if (!m_deliveryBox.isBeingCarried())
    {
        for (const DeliveryBox::DeliveryBoxPart& part : m_deliveryBox.parts())
        {
            m_carryColliders.push_back(AABB::fromCenterAndSize(part.center, part.size));
        }
    }

    if (m_interaction == nullptr)
    {
        return;
    }

    m_interaction->updateLidColliders(m_backLidWorldVertices, m_frontLidWorldVertices);

    if (!m_deliveryBox.isBeingCarried() && m_interaction->hasLidColliders())
    {
        m_carryColliders.push_back(m_interaction->backLidCollider());

        m_carryColliders.push_back(m_interaction->frontLidCollider());
    }
}

void DeliveryBoxScene::updateBodyPhysicsTransforms()
{
    const DeliveryBoxGeometry& geometry = m_deliveryBox.geometry();

    const Matrix& boxWorldTransform = m_deliveryBox.worldTransform();

    float boxWorldScale = m_deliveryBox.worldScale();

    for (std::size_t index = 0; index < geometry.parts.size(); ++index)
    {
        const DeliveryBoxGeometry::Part& part = geometry.parts[index];

        Vector scaledLocalCenter = part.center * boxWorldScale;

        Vector physicsSize = part.size * boxWorldScale;

        if (index == kBottomPartIndex && physicsSize.Y < kMinimumBottomPhysicsThickness)
        {
            float additionalThickness = kMinimumBottomPhysicsThickness - physicsSize.Y;

            // Der Boden wird nur nach unten dicker
            // Seine obere Fläche und damit der Innenraum bleiben unverändert
            scaledLocalCenter.Y -= additionalThickness * 0.5f;

            physicsSize.Y = kMinimumBottomPhysicsThickness;
        }

        Matrix centerOffsetTransform;

        centerOffsetTransform.translation(scaledLocalCenter);

        m_bodyPhysicsTransforms[index] = boxWorldTransform * centerOffsetTransform;

        m_bodyPhysicsSizes[index] = physicsSize;
    }
}

void DeliveryBoxScene::updateLidColliderVertices()
{
    const std::vector<Vector>& localVertices = m_deliveryBox.geometry().lidVertices;

    m_backLidWorldVertices =
        ConvexBoxCollision ::transformVertices(localVertices, m_backLidTransform);

    m_frontLidWorldVertices =
        ConvexBoxCollision ::transformVertices(localVertices, m_frontLidTransform);
}

bool DeliveryBoxScene::isLidBlocked(const std::vector<Product*>& products) const
{
    for (const Product* product : products)
    {
        if (product == nullptr)
        {
            continue;
        }

        const std::vector<Vector>& localVertices = product->localColliderVertices();

        if (localVertices.empty())
        {
            continue;
        }

        std::vector<Vector> productWorldVertices =
            ConvexBoxCollision ::transformVertices(localVertices, product->getWorldTransform());

        bool overlapsBackLid =
            ConvexBoxCollision::overlaps(m_backLidWorldVertices, productWorldVertices);

        bool overlapsFrontLid =
            ConvexBoxCollision::overlaps(m_frontLidWorldVertices, productWorldVertices);

        if (overlapsBackLid || overlapsFrontLid)
        {
            return true;
        }
    }

    return false;
}

Matrix DeliveryBoxScene::createLidPhysicsTransform(const Matrix& lidModelTransform) const
{
    // Der Deckel-Physikcollider besitzt eine lokale Orientierung, die in Weltachsen übertragen werden muss
    const DeliveryBoxGeometry::OrientedBox& collider = m_deliveryBox.geometry().lidPhysicsCollider;

    Vector worldCenter = lidModelTransform * collider.center;

    Vector worldRight = lidModelTransform.transformVec3x3(collider.right);

    Vector worldUp = lidModelTransform.transformVec3x3(collider.up);

    Vector worldForward = lidModelTransform.transformVec3x3(collider.forward);

    if (worldRight.lengthSquared() > 0.000001f)
    {
        worldRight.normalize();
    }

    if (worldUp.lengthSquared() > 0.000001f)
    {
        worldUp.normalize();
    }

    if (worldForward.lengthSquared() > 0.000001f)
    {
        worldForward.normalize();
    }

    Matrix physicsTransform;

    physicsTransform.translation(worldCenter);

    physicsTransform.right(worldRight);

    physicsTransform.up(worldUp);

    physicsTransform.forward(worldForward);

    return physicsTransform;
}

IInteractable* DeliveryBoxScene::interaction() const
{
    return m_interaction.get();
}

std::vector<BaseModel*> DeliveryBoxScene::highlightModels() const
{
    std::vector<BaseModel*> models;

    if (m_bodyModel != nullptr)
    {
        models.push_back(m_bodyModel);
    }

    if (m_backLidModel != nullptr)
    {
        models.push_back(m_backLidModel);
    }

    if (m_frontLidModel != nullptr)
    {
        models.push_back(m_frontLidModel);
    }

    return models;
}
