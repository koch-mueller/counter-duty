#pragma once

#include "../../engine/Aabb.h"
#include "Ray.h"

// Statische Hilfsfunktion für Ray-AABB-Schnittpunkte
class Raycast
{
public:
    static bool rayIntersectsAABB(const Ray& ray, const AABB& aabb, float& hitDistance);
};
