#pragma once

#include "../../engine/Aabb.h"
#include "../../engine/Matrix.h"

#include "Scanner.h"
#include "ScannerVisualState.h"

#include <list>

class AssetLoader;
class BaseModel;
class PhongShader;

// Baut Scanner-Modell und Scan-Zone auf und verwaltet den sichtbaren Scannerzustand
class ScannerStation
{
public:
    explicit ScannerStation(const AssetLoader& assetLoader);

    void create(std::list<BaseModel*>& models,
                std::list<AABB>& playerColliders,
                std::list<AABB>& physicsColliders);

    bool containsBarcode(const Vector& barcodeWorldPosition) const;

    bool isBarcodeAngleValid(const Vector& barcodeWorldNormal) const;

    const AABB& scanZone() const;

    void setVisualState(ScannerVisualState state);

private:
    bool initializeCounterAsset();

    Matrix counterTransform() const;

    BaseModel* createCounterVisual() const;

    void createScannerVisual(std::list<BaseModel*>& models);

    const AssetLoader& m_assetLoader;

    AABB m_counterModelBounds;

    Vector m_counterModelScale;

    AABB m_counterBounds;

    AABB m_scannerSurfaceBounds;

    bool m_counterAssetReady;

    Scanner m_scanner;

    PhongShader* m_shader;

    ScannerVisualState m_visualState;

    bool m_visualStateApplied;
};
