#include "Raycast.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
    bool updateAxisInterval(float origin,
                            float direction,
                            float minimum,
                            float maximum,
                            float& nearestDistance,
                            float& farthestDistance)
    {
        const float epsilon = 0.000001f;

        if (std::abs(direction) < epsilon)
        {
            return origin >= minimum && origin <= maximum;
        }

        float axisNearestDistance = (minimum - origin) / direction;

        float axisFarthestDistance = (maximum - origin) / direction;

        if (axisNearestDistance > axisFarthestDistance)
        {
            std::swap(axisNearestDistance, axisFarthestDistance);
        }

        nearestDistance = std::max(nearestDistance, axisNearestDistance);

        farthestDistance = std::min(farthestDistance, axisFarthestDistance);

        return nearestDistance <= farthestDistance;
    }
}

bool Raycast::rayIntersectsAABB(const Ray& ray, const AABB& aabb, float& hitDistance)
{
    // Slab-Methode: Für jede Achse wird das gültige t-Intervall eingeschränkt. Bleibt kein gemeinsames Intervall, gibt es keinen Treffer
    float nearestDistance = -std::numeric_limits<float>::infinity();

    float farthestDistance = std::numeric_limits<float>::infinity();

    if (!updateAxisInterval(ray.origin.X,
                            ray.direction.X,
                            aabb.Min.X,
                            aabb.Max.X,
                            nearestDistance,
                            farthestDistance))
    {
        return false;
    }

    if (!updateAxisInterval(ray.origin.Y,
                            ray.direction.Y,
                            aabb.Min.Y,
                            aabb.Max.Y,
                            nearestDistance,
                            farthestDistance))
    {
        return false;
    }

    if (!updateAxisInterval(ray.origin.Z,
                            ray.direction.Z,
                            aabb.Min.Z,
                            aabb.Max.Z,
                            nearestDistance,
                            farthestDistance))
    {
        return false;
    }

    if (farthestDistance < 0.0f)
    {
        return false;
    }

    hitDistance = std::max(nearestDistance, 0.0f);

    return true;
}
