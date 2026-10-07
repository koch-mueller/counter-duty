#include "StoreEnvironmentBuilder.h"

#include "../../engine/Lights.h"
#include "../../engine/ShaderLightMapper.h"
#include "../../engine/TriangleBoxModel.h"

#include "../../assets/AssetLoader.h"

#include "../../engine/Model.h"
#include "../../engine/PhongShader.h"

#include "../../engine/TexturedPlaneModel.h"
#include "../../engine/Texture.h"

#include "StoreLayout.h"

#include <array>

namespace
{
    constexpr float kPi = 3.14159265358979323846f;

    constexpr float kMinimumModelDimension = 0.000001f;

    const char* kTerminalModelPath = "local/models/environment/terminal/terminal.glb";

    const char* kTerminalBaseColorPath =
        "local/models/environment/terminal/textures/terminal_g_clear_BaseColor.png";

    const char* kTerminalNormalPath =
        "local/models/environment/terminal/textures/terminal_g_clear_Normal.png";

    const char* kMopStationModelPath = "models/cleaning/mop_station.obj";

    constexpr float kMopStationTargetHeight = 0.6f;

    const char* kMopModelPath = "models/cleaning/cleaning_mop.obj";

    constexpr float kMopTargetHeight = 1.35f;

    const char* kSpillDiffuseTexturePath = "materials/spill/spill_diffuse.jpg";

    const char* kSpillNormalTexturePath = "materials/spill/spill_diffuse_n.png";

    const char* kSpillOpacityTexturePath = "materials/spill/spill_opacity.png";

    const char* kCeilingLightModelPath =
        "local/models/environment/lights/fluorescent_light.obj";

    constexpr float kCeilingLightTargetWidth = 0.9f;

    PhongShader* createColoredShader(const Color& color)
    {
        PhongShader* shader = new PhongShader();

        shader->diffuseColor(color);
        shader->ambientColor(color * 0.25f);

        shader->specularColor(Color(0.08f, 0.08f, 0.08f));

        shader->specularExp(10.0f);

        return shader;
    }

    TriangleBoxModel* createBoxModel(const Vector& center, const Vector& size, const Color& color)
    {
        TriangleBoxModel* model = new TriangleBoxModel(size.X, size.Y, size.Z);

        Matrix transform;

        transform.translation(center);

        model->transform(transform);

        model->shader(createColoredShader(color), true);

        return model;
    }

    BaseModel* createMaterialBoxModel(const Vector& center,
                                      const Vector& size,
                                      const PhongMaterial& material)
    {
        TriangleBoxModel* model = new TriangleBoxModel(size.X, size.Y, size.Z);

        Matrix transform;

        transform.translation(center);

        model->transform(transform);

        model->shader(material.createShader(), true);

        return model;
    }
}

StoreEnvironmentBuilder::StoreEnvironmentBuilder(const AssetLoader& assetLoader)
    : m_assetLoader(assetLoader),
      m_terminalVisual(nullptr),
      m_terminalModelBounds(),
      m_terminalModelScale(1.0f, 1.0f, 1.0f),
      m_terminalBounds(),
      m_terminalAssetReady(false),
      m_floorMaterial(),
      m_wallMaterial(),
      m_ceilingMaterial(),
      m_deliveryZoneMaterial()
{
    Vector terminalSize(0.8f, StoreLayout::terminalHeight(), 0.8f);

    m_terminalBounds = AABB::fromCenterAndSize(StoreLayout::terminalCenter(), terminalSize);

    m_terminalAssetReady = initializeTerminalAsset();

    // Flächenmaterialien werden einmal beim Aufbau geladen und anschließend für alle passenden Modelle wiederverwendet
    bool floorLoaded = m_floorMaterial.load(assetLoader,
                                            "materials/surfaces/"
                                            "floor_tiles/"
                                            "floor_tiles.mtl");

    if (!floorLoaded)
    {
        std::cerr << "Failed to load floor material\n";
    }

    bool wallLoaded = m_wallMaterial.load(assetLoader,
                                          "materials/surfaces/"
                                          "beige_wall/"
                                          "beige_wall.mtl");

    if (!wallLoaded)
    {
        std::cerr << "Failed to load wall material\n";
    }

    bool ceilingLoaded = m_ceilingMaterial.load(assetLoader,
                                                "materials/surfaces/"
                                                "ceiling/"
                                                "ceiling.mtl");

    if (!ceilingLoaded)
    {
        std::cerr << "Failed to load ceiling material\n";
    }

    bool deliveryZoneLoaded = m_deliveryZoneMaterial.load(assetLoader,
                                                          "materials/delivery_zone/"
                                                          "delivery_zone.mtl");

    if (!deliveryZoneLoaded)
    {
        std::cerr << "Failed to load delivery zone material\n";
    }
}

bool StoreEnvironmentBuilder::initializeTerminalAsset()
{
    if (!m_assetLoader.loadModelBounds(kTerminalModelPath, m_terminalModelBounds))
    {
        return false;
    }

    Vector modelSize = m_terminalModelBounds.size();

    if (modelSize.X <= kMinimumModelDimension || modelSize.Y <= kMinimumModelDimension
        || modelSize.Z <= kMinimumModelDimension)
    {
        return false;
    }

    float uniformScale = StoreLayout::terminalHeight() / modelSize.Y;

    m_terminalModelScale = Vector(uniformScale, uniformScale, uniformScale);

    Vector terminalSize(modelSize.X * uniformScale,
                        modelSize.Y * uniformScale,
                        modelSize.Z * uniformScale);

    m_terminalBounds = AABB::fromCenterAndSize(StoreLayout::terminalCenter(), terminalSize);

    return true;
}

void StoreEnvironmentBuilder::create(std::list<BaseModel*>& models,
                                     std::list<AABB>& playerColliders,
                                     std::list<AABB>& interactionColliders,
                                     std::list<AABB>& physicsColliders)
{
    createFloor(models, physicsColliders);

    createRoof(models);

    createWalls(models, playerColliders, interactionColliders, physicsColliders);

    createDeliveryArea(models);

    createTerminal(models, playerColliders, physicsColliders);

    createMopStation(models, playerColliders, physicsColliders);

    createCeilingLights(models);

    createLights();
}

void StoreEnvironmentBuilder::createFloor(std::list<BaseModel*>& models,
                                          std::list<AABB>& physicsColliders) const
{
    Vector floorCenter = StoreLayout::floorCenter();

    Vector floorSize = StoreLayout::floorSize();

    // Texturwiederholungen werden aus der Weltgröße berechnet, damit die Fliesen unabhängig von der Raumgröße gleich groß wirken
    constexpr float materialSize = 1.9f;

    float repeatU = floorSize.X / materialSize;

    float repeatV = floorSize.Z / materialSize;

    TexturedPlaneModel* floorModel =
        new TexturedPlaneModel(floorSize.X, floorSize.Z, repeatU, repeatV, PlaneOrientation::XZ);

    Matrix transform;

    transform.translation(floorCenter.X, floorCenter.Y + floorSize.Y * 0.5f, floorCenter.Z);

    floorModel->transform(transform);

    floorModel->shader(m_floorMaterial.createShader(), true);

    models.push_back(floorModel);

    physicsColliders.push_back(AABB::fromCenterAndSize(floorCenter, floorSize));
}

void StoreEnvironmentBuilder::createRoof(std::list<BaseModel*>& models) const
{
    Vector roofCenter = StoreLayout::roofCenter();

    Vector roofSize = StoreLayout::roofSize();

    constexpr float materialSize = 2.0f;

    float repeatU = roofSize.X / materialSize;

    float repeatV = roofSize.Z / materialSize;

    TexturedPlaneModel* roofModel = new TexturedPlaneModel(roofSize.X,
                                                           roofSize.Z,
                                                           repeatU,
                                                           repeatV,
                                                           PlaneOrientation::XZ,
                                                           true);

    Matrix transform;

    transform.translation(roofCenter.X, roofCenter.Y - roofSize.Y * 0.5f, roofCenter.Z);

    roofModel->transform(transform);

    roofModel->shader(m_ceilingMaterial.createShader(), true);

    models.push_back(roofModel);
}

void StoreEnvironmentBuilder::createWalls(std::list<BaseModel*>& models,
                                          std::list<AABB>& playerColliders,
                                          std::list<AABB>& interactionColliders,
                                          std::list<AABB>& physicsColliders) const
{
    Vector frontCenter = StoreLayout::frontWallCenter();

    Vector backCenter = StoreLayout::backWallCenter();

    Vector leftCenter = StoreLayout::leftWallCenter();

    Vector rightCenter = StoreLayout::rightWallCenter();

    Vector frontBackSize = StoreLayout::frontBackWallSize();

    Vector sideSize = StoreLayout::sideWallSize();

    constexpr float materialSize = 2.0f;

    float frontBackRepeatU = frontBackSize.X / materialSize;

    float frontBackRepeatV = frontBackSize.Y / materialSize;

    float sideRepeatU = sideSize.Z / materialSize;

    float sideRepeatV = sideSize.Y / materialSize;

    TexturedPlaneModel* frontWall = new TexturedPlaneModel(frontBackSize.X,
                                                           frontBackSize.Y,
                                                           frontBackRepeatU,
                                                           frontBackRepeatV,
                                                           PlaneOrientation::XY);

    Matrix frontTransform;

    frontTransform.translation(frontCenter.X,
                               frontCenter.Y,
                               frontCenter.Z + frontBackSize.Z * 0.5f);

    frontWall->transform(frontTransform);

    frontWall->shader(m_wallMaterial.createShader(), true);

    models.push_back(frontWall);

    TexturedPlaneModel* backWall = new TexturedPlaneModel(frontBackSize.X,
                                                          frontBackSize.Y,
                                                          frontBackRepeatU,
                                                          frontBackRepeatV,
                                                          PlaneOrientation::XY,
                                                          true);

    Matrix backTransform;

    backTransform.translation(backCenter.X, backCenter.Y, backCenter.Z - frontBackSize.Z * 0.5f);

    backWall->transform(backTransform);

    backWall->shader(m_wallMaterial.createShader(), true);

    models.push_back(backWall);

    TexturedPlaneModel* leftWall = new TexturedPlaneModel(sideSize.Z,
                                                          sideSize.Y,
                                                          sideRepeatU,
                                                          sideRepeatV,
                                                          PlaneOrientation::YZ);

    Matrix leftTransform;

    leftTransform.translation(leftCenter.X + sideSize.X * 0.5f, leftCenter.Y, leftCenter.Z);

    leftWall->transform(leftTransform);

    leftWall->shader(m_wallMaterial.createShader(), true);

    models.push_back(leftWall);

    TexturedPlaneModel* rightWall = new TexturedPlaneModel(sideSize.Z,
                                                           sideSize.Y,
                                                           sideRepeatU,
                                                           sideRepeatV,
                                                           PlaneOrientation::YZ,
                                                           true);

    Matrix rightTransform;

    rightTransform.translation(rightCenter.X - sideSize.X * 0.5f, rightCenter.Y, rightCenter.Z);

    rightWall->transform(rightTransform);

    rightWall->shader(m_wallMaterial.createShader(), true);

    models.push_back(rightWall);

    const std::array<Vector, 4> wallCenters = {frontCenter, backCenter, leftCenter, rightCenter};

    const std::array<Vector, 4> wallSizes = {frontBackSize, frontBackSize, sideSize, sideSize};

    for (std::size_t index = 0; index < wallCenters.size(); ++index)
    {
        AABB collider = AABB::fromCenterAndSize(wallCenters[index], wallSizes[index]);

        playerColliders.push_back(collider);

        interactionColliders.push_back(collider);

        physicsColliders.push_back(collider);
    }
}

void StoreEnvironmentBuilder::createDeliveryArea(std::list<BaseModel*>& models) const
{
    Vector center = StoreLayout::deliveryAreaCenter();

    Vector size = StoreLayout::deliveryAreaSize();

    constexpr float borderWidth = 0.08f;

    constexpr float borderHeight = 0.012f;

    float halfWidth = size.X * 0.5f;

    float halfDepth = size.Z * 0.5f;

    Vector frontCenter(center.X, borderHeight * 0.5f + 0.002f, center.Z - halfDepth);

    Vector backCenter(center.X, borderHeight * 0.5f + 0.002f, center.Z + halfDepth);

    Vector leftCenter(center.X - halfWidth, borderHeight * 0.5f + 0.002f, center.Z);

    Vector rightCenter(center.X + halfWidth, borderHeight * 0.5f + 0.002f, center.Z);

    Vector frontBackSize(size.X + borderWidth, borderHeight, borderWidth);

    Vector sideSize(borderWidth, borderHeight, size.Z - borderWidth);

    models.push_back(createMaterialBoxModel(frontCenter, frontBackSize, m_deliveryZoneMaterial));

    models.push_back(createMaterialBoxModel(backCenter, frontBackSize, m_deliveryZoneMaterial));

    models.push_back(createMaterialBoxModel(leftCenter, sideSize, m_deliveryZoneMaterial));

    models.push_back(createMaterialBoxModel(rightCenter, sideSize, m_deliveryZoneMaterial));
}

void StoreEnvironmentBuilder::createTerminal(std::list<BaseModel*>& models,
                                             std::list<AABB>& playerColliders,
                                             std::list<AABB>& physicsColliders)
{
    const Vector center = StoreLayout::terminalCenter();

    m_terminalVisual = nullptr;

    if (m_terminalAssetReady)
    {
        Model* terminalModel = m_assetLoader.loadModel(kTerminalModelPath, false);

        if (terminalModel != nullptr)
        {
            terminalModel->diffuseTextureOverride(
                Texture::LoadShared(m_assetLoader.assetPath(kTerminalBaseColorPath).c_str()));

            terminalModel->normalTextureOverride(
                Texture::LoadShared(m_assetLoader.assetPath(kTerminalNormalPath).c_str()));

            Matrix centerOffsetTransform;
            centerOffsetTransform.translation(-m_terminalModelBounds.center());

            Matrix scaleTransform;
            scaleTransform.scale(m_terminalModelScale);

            Matrix translationTransform;
            translationTransform.translation(center);

            terminalModel->transform(translationTransform * scaleTransform * centerOffsetTransform);
            terminalModel->shader(new PhongShader(), true);

            m_terminalVisual = terminalModel;
        }
    }

    if (m_terminalVisual == nullptr)
    {
        m_terminalVisual =
            createBoxModel(Vector(center.X, 0.68f, center.Z),
                           Vector(0.56f, 1.24f, 0.52f),
                           Color(0.18f, 0.20f, 0.22f));

        models.push_back(createBoxModel(Vector(center.X, 0.08f, center.Z),
                                        Vector(0.76f, 0.16f, 0.72f),
                                        Color(0.08f, 0.09f, 0.10f)));

        models.push_back(createBoxModel(Vector(center.X, 1.47f, center.Z + 0.04f),
                                        Vector(0.78f, 0.54f, 0.42f),
                                        Color(0.12f, 0.14f, 0.16f)));

        TriangleBoxModel* screen =
            createBoxModel(Vector(center.X, 1.50f, center.Z + 0.265f),
                           Vector(0.60f, 0.34f, 0.035f),
                           Color(0.10f, 0.78f, 0.82f));

        screen->shadowCaster(false);

        models.push_back(screen);

        models.push_back(createBoxModel(Vector(center.X, 1.12f, center.Z + 0.23f),
                                        Vector(0.55f, 0.12f, 0.30f),
                                        Color(0.82f, 0.50f, 0.16f)));
    }

    models.push_back(m_terminalVisual);

    playerColliders.push_back(m_terminalBounds);

    physicsColliders.push_back(m_terminalBounds);
}

Model* StoreEnvironmentBuilder::createCleaningToolVisual(Matrix& visualOffset,
                                                         Vector& toolSize) const
{
    AABB modelBounds;

    if (!m_assetLoader.loadModelBounds(kMopModelPath, modelBounds))
    {
        return nullptr;
    }

    Vector modelSize = modelBounds.size();

    if (modelSize.Y <= kMinimumModelDimension)
    {
        return nullptr;
    }

    float uniformScale = kMopTargetHeight / modelSize.Y;

    toolSize =
        Vector(modelSize.X * uniformScale, modelSize.Y * uniformScale, modelSize.Z * uniformScale);

    Model* mopModel = m_assetLoader.loadModel(kMopModelPath, false);

    if (mopModel == nullptr)
    {
        return nullptr;
    }

    Matrix centerOffsetTransform;

    centerOffsetTransform.translation(-modelBounds.center());

    Matrix scaleTransform;

    scaleTransform.scale(Vector(uniformScale, uniformScale, uniformScale));

    visualOffset = scaleTransform * centerOffsetTransform;

    mopModel->shader(new PhongShader(), true);

    return mopModel;
}

BaseModel* StoreEnvironmentBuilder::createSpillVisual() const
{
    TexturedPlaneModel* spillModel =
        new TexturedPlaneModel(1.0f, 1.0f, 1.0f, 1.0f, PlaneOrientation::XZ);

    PhongShader* shader = new PhongShader();

    shader->diffuseColor(Color(1.0f, 1.0f, 1.0f));

    shader->ambientColor(Color(0.2f, 0.2f, 0.2f));

    shader->specularColor(Color(0.2f, 0.2f, 0.2f));

    shader->specularExp(32.0f);

    shader->diffuseTexture(
        Texture::LoadShared(m_assetLoader.assetPath(kSpillDiffuseTexturePath).c_str()));

    shader->normalTexture(
        Texture::LoadShared(m_assetLoader.assetPath(kSpillNormalTexturePath).c_str()));

    // Die Opacity-Textur entfernt die rechteckigen Ränder des Plane-Modells
    shader->opacityTexture(
        Texture::LoadShared(m_assetLoader.assetPath(kSpillOpacityTexturePath).c_str()));

    spillModel->shader(shader, true);

    spillModel->shadowCaster(false);

    return spillModel;
}

void StoreEnvironmentBuilder::createMopStation(std::list<BaseModel*>& models,
                                               std::list<AABB>& playerColliders,
                                               std::list<AABB>& physicsColliders) const
{
    AABB modelBounds;

    if (!m_assetLoader.loadModelBounds(kMopStationModelPath, modelBounds))
    {
        return;
    }

    Vector modelSize = modelBounds.size();

    if (modelSize.Y <= 0.000001f)
    {
        return;
    }

    float uniformScale = kMopStationTargetHeight / modelSize.Y;

    Vector stationSize = modelSize * uniformScale;

    Vector stationCenter = StoreLayout::cleaningStationCenter();

    stationCenter.Y = stationSize.Y * 0.5f;

    Model* stationModel = m_assetLoader.loadModel(kMopStationModelPath, false);

    if (stationModel == nullptr)
    {
        return;
    }

    Matrix centerOffsetTransform;

    centerOffsetTransform.translation(-modelBounds.center());

    Matrix scaleTransform;

    scaleTransform.scale(Vector(uniformScale, uniformScale, uniformScale));

    Matrix translationTransform;

    translationTransform.translation(stationCenter);

    stationModel->transform(translationTransform * scaleTransform * centerOffsetTransform);

    stationModel->shader(new PhongShader(), true);

    models.push_back(stationModel);

    AABB stationBounds = AABB::fromCenterAndSize(stationCenter, stationSize);

    playerColliders.push_back(stationBounds);

    physicsColliders.push_back(stationBounds);
}

void StoreEnvironmentBuilder::createLights() const
{
    // Jede sichtbare Deckenlampe erhält ein passendes SpotLight; zusätzliche schwache PointLights hellen harte Schattenbereiche auf
    for (const Vector& lampPosition : StoreLayout::ceilingLightCenters())
    {
        Vector lightPosition = lampPosition;
        lightPosition.Y -= 0.12f;

        SpotLight* light = new SpotLight(lightPosition,
                                         Vector(0.0f, -1.0f, 0.0f),
                                         55.0f,
                                         75.0f,
                                         Color(0.9f, 0.95f, 1.0f));

        light->attenuation(Vector(1.0f, 0.05f, 0.08f));

        ShaderLightMapper::instance().addLight(light);
    }

    PointLight* frontBounceLight =
        new PointLight(Vector(0.0f, -0.25f, -2.5f), Color(0.12f, 0.13f, 0.15f));

    frontBounceLight->attenuation(Vector(1.0f, 0.08f, 0.035f));

    ShaderLightMapper::instance().addLight(frontBounceLight);

    PointLight* backBounceLight =
        new PointLight(Vector(0.0f, -0.25f, 2.5f), Color(0.12f, 0.13f, 0.15f));

    backBounceLight->attenuation(Vector(1.0f, 0.08f, 0.035f));

    ShaderLightMapper::instance().addLight(backBounceLight);
}

void StoreEnvironmentBuilder::createCeilingLights(std::list<BaseModel*>& models) const
{
    AABB modelBounds;

    if (m_assetLoader.loadModelBounds(kCeilingLightModelPath, modelBounds))
    {
        Vector modelSize = modelBounds.size();

        if (modelSize.X > kMinimumModelDimension)
        {
            float uniformScale = kCeilingLightTargetWidth / modelSize.X;

            Matrix centerOffsetTransform;
            centerOffsetTransform.translation(-modelBounds.center());

            Matrix rotationTransform;
            rotationTransform.rotationX(kPi);

            Matrix scaleTransform;
            scaleTransform.scale(Vector(uniformScale, uniformScale, uniformScale));

            for (const Vector& position : StoreLayout::ceilingLightCenters())
            {
                Model* lightModel = m_assetLoader.loadModel(kCeilingLightModelPath, false);

                if (lightModel == nullptr)
                {
                    continue;
                }

                Matrix translationTransform;
                translationTransform.translation(position);

                lightModel->transform(translationTransform * rotationTransform * scaleTransform
                                      * centerOffsetTransform);
                lightModel->shader(new PhongShader(), true);
                lightModel->shadowCaster(false);

                models.push_back(lightModel);
            }

            return;
        }
    }

    for (const Vector& position : StoreLayout::ceilingLightCenters())
    {
        TriangleBoxModel* housing =
            createBoxModel(position, Vector(1.08f, 0.10f, 0.38f), Color(0.24f, 0.25f, 0.27f));

        housing->shadowCaster(false);

        models.push_back(housing);

        TriangleBoxModel* lightPanel =
            createBoxModel(Vector(position.X, position.Y - 0.065f, position.Z),
                           Vector(0.92f, 0.035f, 0.25f),
                           Color(0.92f, 0.96f, 1.0f));

        lightPanel->shadowCaster(false);

        models.push_back(lightPanel);
    }
}

const AABB& StoreEnvironmentBuilder::terminalBounds() const
{
    return m_terminalBounds;
}

BaseModel* StoreEnvironmentBuilder::terminalVisual() const
{
    return m_terminalVisual;
}
