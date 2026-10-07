#pragma once

#include "../../engine/Aabb.h"

#include <array>

namespace StoreLayout
{
    Vector floorCenter();

    Vector floorSize();

    Vector roofCenter();

    Vector roofSize();

    Vector frontWallCenter();

    Vector backWallCenter();

    Vector leftWallCenter();

    Vector rightWallCenter();

    Vector frontBackWallSize();

    Vector sideWallSize();

    Vector checkoutPosition();

    Vector terminalCenter();

    float terminalHeight();

    Vector deliveryAreaCenter();

    Vector deliveryAreaSize();

    AABB deliveryZoneBounds();

    float deliveryZoneInteractionDistance();

    Vector deliveryBoxResetPosition();

    Vector cleaningSpotSize();

    const std::array<Vector, 2>& cleaningSpotCenters();

    Vector cleaningToolCenter();

    Vector cleaningStationCenter();

    Vector playerStartPosition();

    const std::array<Vector, 2>& shelfCenters();

    const std::array<float, 2>& shelfFrontDirections();

    const std::array<Vector, 6>& ceilingLightCenters();

}
