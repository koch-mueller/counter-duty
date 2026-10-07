#include "StoreLayout.h"

namespace
{
    constexpr float kStoreWidth = 14.0f;

    constexpr float kStoreDepth = 12.0f;

    constexpr float kStoreHeight = 4.0f;

    constexpr float kWallThickness = 0.1f;

    const std::array<Vector, 2> kCleaningSpotCenters = {Vector(-3.2f, 0.005f, -2.2f),
                                                        Vector(2.4f, 0.005f, 2.4f)};

    const std::array<Vector, 2> kShelfCenters = {Vector(0.0f, 1.05f, 0.435f),
                                                 Vector(0.0f, 1.05f, -0.435f)};

    const std::array<float, 2> kShelfFrontDirections = {1.0f, -1.0f};

    const std::array<Vector, 6> kCeilingLightCenters = {Vector(-3.5f, 3.85f, -3.5f),
                                                        Vector(3.5f, 3.85f, -3.5f),
                                                        Vector(-3.5f, 3.85f, 0.0f),
                                                        Vector(3.5f, 3.85f, 0.0f),
                                                        Vector(-3.5f, 3.85f, 3.5f),
                                                        Vector(3.5f, 3.85f, 3.5f)};

}

Vector StoreLayout::floorCenter()
{
    return Vector(0.0f, -0.05f, 0.0f);
}

Vector StoreLayout::floorSize()
{
    return Vector(kStoreWidth, 0.1f, kStoreDepth);
}

Vector StoreLayout::roofCenter()
{
    return Vector(0.0f, kStoreHeight + 0.05f, 0.0f);
}

Vector StoreLayout::roofSize()
{
    return Vector(kStoreWidth, 0.1f, kStoreDepth);
}

Vector StoreLayout::frontWallCenter()
{
    return Vector(0.0f, kStoreHeight * 0.5f, -kStoreDepth * 0.5f);
}

Vector StoreLayout::backWallCenter()
{
    return Vector(0.0f, kStoreHeight * 0.5f, kStoreDepth * 0.5f);
}

Vector StoreLayout::leftWallCenter()
{
    return Vector(-kStoreWidth * 0.5f, kStoreHeight * 0.5f, 0.0f);
}

Vector StoreLayout::rightWallCenter()
{
    return Vector(kStoreWidth * 0.5f, kStoreHeight * 0.5f, 0.0f);
}

Vector StoreLayout::frontBackWallSize()
{
    return Vector(kStoreWidth, kStoreHeight, kWallThickness);
}

Vector StoreLayout::sideWallSize()
{
    return Vector(kWallThickness, kStoreHeight, kStoreDepth);
}

Vector StoreLayout::checkoutPosition()
{
    return Vector(2.8f, 0.0f, -3.8f);
}

Vector StoreLayout::terminalCenter()
{
    return Vector(-4.8f, 0.9f, -4.85f);
}

float StoreLayout::terminalHeight()
{
    return 1.8f;
}

Vector StoreLayout::deliveryAreaCenter()
{
    return Vector(-4.6f, 0.005f, 4.2f);
}

Vector StoreLayout::deliveryAreaSize()
{
    return Vector(2.6f, 0.01f, 2.4f);
}

AABB StoreLayout::deliveryZoneBounds()
{
    Vector center = deliveryAreaCenter();
    Vector size = deliveryAreaSize();
    Vector halfSize = size * 0.5f;

    return AABB(Vector(center.X - halfSize.X, -0.05f, center.Z - halfSize.Z),
                Vector(center.X + halfSize.X, 2.5f, center.Z + halfSize.Z));
}

float StoreLayout::deliveryZoneInteractionDistance()
{
    return 2.0f;
}

Vector StoreLayout::deliveryBoxResetPosition()
{
    return Vector(4.8f, 0.0f, -1.4f);
}

Vector StoreLayout::cleaningSpotSize()
{
    return Vector(1.0f, 0.01f, 1.0f);
}

const std::array<Vector, 2>& StoreLayout::cleaningSpotCenters()
{
    return kCleaningSpotCenters;
}

Vector StoreLayout::cleaningToolCenter()
{
    return Vector(6.1f, 0.5f, 4.6f);
}

Vector StoreLayout::cleaningStationCenter()
{
    return Vector(5.4f, 0.0f, 4.6f);
}

Vector StoreLayout::playerStartPosition()
{
    return Vector(0.0f, 1.7f, -4.9f);
}

const std::array<Vector, 2>& StoreLayout::shelfCenters()
{
    return kShelfCenters;
}

const std::array<float, 2>& StoreLayout::shelfFrontDirections()
{
    return kShelfFrontDirections;
}

const std::array<Vector, 6>& StoreLayout::ceilingLightCenters()
{
    return kCeilingLightCenters;
}
