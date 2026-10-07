#include "ScannerStation.h"

#include "../../assets/AssetLoader.h"

#include "../../engine/Model.h"
#include "../../engine/PhongShader.h"
#include "../../engine/TriangleBoxModel.h"

#include "../scene/StoreLayout.h"

#include <iostream>

namespace
{
    constexpr float kMaximumScanAngle = 40.0f;

    constexpr float kMinimumModelDimension = 0.000001f;

    const char* kCounterModelPath = "models/environment/checkout/scene.gltf";

    const Vector kCounterTargetSize(3.6f, 1.0f, 1.35f);

    const Vector kScannerSize(0.62f, 0.025f, 0.42f);

    constexpr float kScannerFrameWidth = 0.035f;

    constexpr float kScannerGlassHeight = 0.008f;

    constexpr float kScannerScanZoneHeight = 0.24f;

    constexpr float kScannerSurfaceOffset = 0.002f;

    Vector scannerGlassSize()
    {
        return Vector(kScannerSize.X - kScannerFrameWidth * 2.0f,
                      kScannerGlassHeight,
                      kScannerSize.Z - kScannerFrameWidth * 2.0f);
    }

    TriangleBoxModel* createScannerPart(const Vector& center,
                                        const Vector& size,
                                        const Color& color)
    {
        TriangleBoxModel* model = new TriangleBoxModel(size.X, size.Y, size.Z);

        Matrix transform;

        transform.translation(center);

        model->transform(transform);

        model->shadowCaster(false);

        PhongShader* shader = new PhongShader();

        shader->diffuseColor(color);

        shader->ambientColor(color * 0.35f);

        shader->specularColor(Color(0.15f, 0.15f, 0.15f));

        shader->specularExp(25.0f);

        model->shader(shader, true);

        return model;
    }

    Vector counterWorldCenter()
    {
        Vector position = StoreLayout::checkoutPosition();

        return Vector(position.X, position.Y + kCounterTargetSize.Y * 0.5f, position.Z);
    }

    AABB initialScanZone()
    {
        Vector counterPosition = StoreLayout::checkoutPosition();

        return AABB::fromCenterAndSize(Vector(counterPosition.X, 1.15f, counterPosition.Z + 0.25f),
                                       Vector(0.9f, 0.22f, 0.8f));
    }

    Color colorForState(ScannerVisualState state)
    {
        switch (state)
        {
            case ScannerVisualState::Idle:
                return Color(0.05f, 0.12f, 0.15f);

            case ScannerVisualState::Ready:
                return Color(0.1f, 0.7f, 0.9f);

            case ScannerVisualState::Success:
                return Color(0.1f, 0.9f, 0.2f);

            case ScannerVisualState::Error:
                return Color(0.9f, 0.1f, 0.1f);
        }

        return Color(0.05f, 0.12f, 0.15f);
    }
}

ScannerStation::ScannerStation(const AssetLoader& assetLoader)
    : m_assetLoader(assetLoader),
      m_counterModelBounds(),
      m_counterModelScale(1.0f, 1.0f, 1.0f),
      m_counterBounds(AABB::fromCenterAndSize(counterWorldCenter(), kCounterTargetSize)),
      m_scannerSurfaceBounds(),
      m_counterAssetReady(false),
      m_scanner(initialScanZone(), Vector(0.0f, 1.0f, 0.0f), kMaximumScanAngle),
      m_shader(nullptr),
      m_visualState(ScannerVisualState::Idle),
      m_visualStateApplied(false)
{
    m_counterAssetReady = initializeCounterAsset();
}

bool ScannerStation::initializeCounterAsset()
{
    if (!m_assetLoader.loadModelBounds(kCounterModelPath, m_counterModelBounds))
    {
        std::cerr << "Failed to load counter bounds: " << kCounterModelPath << "\n";

        return false;
    }

    Vector modelSize = m_counterModelBounds.size();

    if (modelSize.X <= kMinimumModelDimension || modelSize.Y <= kMinimumModelDimension
        || modelSize.Z <= kMinimumModelDimension)
    {
        std::cerr << "Counter model has invalid bounds: " << kCounterModelPath << "\n";

        return false;
    }

    // Modell, Collider und Scannerposition werden aus denselben Asset-Bounds abgeleitet und bleiben dadurch deckungsgleich
    m_counterModelScale = Vector(kCounterTargetSize.X / modelSize.X,
                                 kCounterTargetSize.Y / modelSize.Y,
                                 kCounterTargetSize.Z / modelSize.Z);

    Matrix transform = counterTransform();

    m_counterBounds = m_counterModelBounds.transform(transform);

    Vector scannerCenter = m_counterBounds.center();

    scannerCenter.Y = m_counterBounds.Max.Y + kScannerSurfaceOffset + kScannerSize.Y * 0.5f;

    m_scannerSurfaceBounds = AABB::fromCenterAndSize(scannerCenter, kScannerSize);

    Vector glassSize = scannerGlassSize();

    // Die unsichtbare Scan-Zone liegt direkt über dem Glas und ist etwas höher als die sichtbare Scannerfläche
    Vector scanZoneSize(glassSize.X, kScannerScanZoneHeight, glassSize.Z);

    Vector scanZoneCenter = scannerCenter;

    scanZoneCenter.Y = m_scannerSurfaceBounds.Max.Y + scanZoneSize.Y * 0.5f;

    m_scanner = Scanner(AABB::fromCenterAndSize(scanZoneCenter, scanZoneSize),
                        Vector(0.0f, 1.0f, 0.0f),
                        kMaximumScanAngle);

    return true;
}

Matrix ScannerStation::counterTransform() const
{
    Matrix centerOffsetTransform;

    centerOffsetTransform.translation(-m_counterModelBounds.center());

    Matrix scaleTransform;

    scaleTransform.scale(m_counterModelScale);

    Matrix translationTransform;

    translationTransform.translation(counterWorldCenter());

    return translationTransform * scaleTransform * centerOffsetTransform;
}

BaseModel* ScannerStation::createCounterVisual() const
{
    if (!m_counterAssetReady)
    {
        return nullptr;
    }

    Model* counterModel = m_assetLoader.loadModel(kCounterModelPath, false);

    if (counterModel == nullptr)
    {
        return nullptr;
    }

    counterModel->transform(counterTransform());

    counterModel->shader(new PhongShader(), true);

    return counterModel;
}

void ScannerStation::create(std::list<BaseModel*>& models,
                            std::list<AABB>& playerColliders,
                            std::list<AABB>& physicsColliders)
{
    if (!m_counterAssetReady)
    {
        std::cerr << "Scanner station was not created "
                     "because the counter asset is unavailable\n";

        return;
    }

    BaseModel* counterVisual = createCounterVisual();

    if (counterVisual == nullptr)
    {
        std::cerr << "Failed to create counter visual: " << kCounterModelPath << "\n";

        m_counterAssetReady = false;

        return;
    }

    models.push_back(counterVisual);

    createScannerVisual(models);

    playerColliders.push_back(m_counterBounds);

    physicsColliders.push_back(m_counterBounds);

    physicsColliders.push_back(m_scannerSurfaceBounds);
}

void ScannerStation::createScannerVisual(std::list<BaseModel*>& models)
{
    Vector scannerCenter = m_scannerSurfaceBounds.center();

    Vector glassSize = scannerGlassSize();

    Color frameColor(0.025f, 0.025f, 0.03f);

    float sideOffsetX = kScannerSize.X * 0.5f - kScannerFrameWidth * 0.5f;

    float sideOffsetZ = kScannerSize.Z * 0.5f - kScannerFrameWidth * 0.5f;

    Vector leftFrameCenter = scannerCenter + Vector(-sideOffsetX, 0.0f, 0.0f);

    Vector rightFrameCenter = scannerCenter + Vector(sideOffsetX, 0.0f, 0.0f);

    Vector frontFrameCenter = scannerCenter + Vector(0.0f, 0.0f, -sideOffsetZ);

    Vector backFrameCenter = scannerCenter + Vector(0.0f, 0.0f, sideOffsetZ);

    // Der eigentliche Scanner wird aus einfachen Boxen aufgebaut; nur der Kassentisch stammt aus einem Asset
    models.push_back(createScannerPart(leftFrameCenter,
                                       Vector(kScannerFrameWidth, kScannerSize.Y, kScannerSize.Z),
                                       frameColor));

    models.push_back(createScannerPart(rightFrameCenter,
                                       Vector(kScannerFrameWidth, kScannerSize.Y, kScannerSize.Z),
                                       frameColor));

    models.push_back(createScannerPart(frontFrameCenter,
                                       Vector(glassSize.X, kScannerSize.Y, kScannerFrameWidth),
                                       frameColor));

    models.push_back(createScannerPart(backFrameCenter,
                                       Vector(glassSize.X, kScannerSize.Y, kScannerFrameWidth),
                                       frameColor));

    Vector glassCenter = scannerCenter;

    glassCenter.Y = m_scannerSurfaceBounds.Max.Y - kScannerGlassHeight * 0.5f - 0.002f;

    TriangleBoxModel* glassModel = new TriangleBoxModel(glassSize.X, glassSize.Y, glassSize.Z);

    Matrix glassTransform;

    glassTransform.translation(glassCenter);

    glassModel->transform(glassTransform);

    glassModel->shadowCaster(false);

    m_shader = new PhongShader();

    glassModel->shader(m_shader, true);

    models.push_back(glassModel);

    m_visualStateApplied = false;

    setVisualState(ScannerVisualState::Idle);

    Vector laserCenter = glassCenter;

    laserCenter.Y = m_scannerSurfaceBounds.Max.Y - 0.0005f;

    Color laserColor(0.9f, 0.02f, 0.02f);

    models.push_back(
        createScannerPart(laserCenter, Vector(glassSize.X * 0.82f, 0.001f, 0.006f), laserColor));

    models.push_back(
        createScannerPart(laserCenter, Vector(0.006f, 0.001f, glassSize.Z * 0.82f), laserColor));
}

bool ScannerStation::containsBarcode(const Vector& barcodeWorldPosition) const
{
    if (!m_counterAssetReady)
    {
        return false;
    }

    return m_scanner.containsBarcode(barcodeWorldPosition);
}

bool ScannerStation::isBarcodeAngleValid(const Vector& barcodeWorldNormal) const
{
    if (!m_counterAssetReady)
    {
        return false;
    }

    return m_scanner.isBarcodeAngleValid(barcodeWorldNormal);
}

const AABB& ScannerStation::scanZone() const
{
    return m_scanner.scanZone();
}

void ScannerStation::setVisualState(ScannerVisualState state)
{
    if (m_shader == nullptr)
    {
        return;
    }

    // Shaderwerte werden nur bei einem echten Zustandswechsel neu gesetzt
    if (m_visualStateApplied && state == m_visualState)
    {
        return;
    }

    m_visualState = state;

    m_visualStateApplied = true;

    Color scannerColor = colorForState(state);

    m_shader->diffuseColor(scannerColor);

    m_shader->ambientColor(scannerColor * 0.4f);

    m_shader->specularColor(Color(0.15f, 0.15f, 0.15f));

    m_shader->specularExp(30.0f);
}
