
#include "DeliveryBoxGeometry.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
    DeliveryBoxGeometry::Part createPart(const AABB& bounds)
    {
        DeliveryBoxGeometry::Part part;

        part.center = bounds.center();

        part.size = bounds.size();

        return part;
    }

    bool positionsAreEqual(const Vector& first, const Vector& second)
    {
        constexpr float tolerance = 0.00001f;

        return std::fabs(first.X - second.X) <= tolerance
               && std::fabs(first.Y - second.Y) <= tolerance
               && std::fabs(first.Z - second.Z) <= tolerance;
    }

    // OBJ-Daten enthalten an Kanten häufig mehrfach identische Positionen. Für die Geometrie reichen eindeutige Punkte
    std::vector<Vector> createUniqueVertices(const std::vector<Vector>& vertices)
    {
        std::vector<Vector> uniqueVertices;

        for (const Vector& vertex : vertices)
        {
            bool alreadyExists = false;

            for (const Vector& uniqueVertex : uniqueVertices)
            {
                if (positionsAreEqual(vertex, uniqueVertex))
                {
                    alreadyExists = true;

                    break;
                }
            }

            if (!alreadyExists)
            {
                uniqueVertices.push_back(vertex);
            }
        }

        return uniqueVertices;
    }

    DeliveryBoxGeometry::OrientedBox createLidPhysicsCollider(const std::vector<Vector>& vertices,
                                                              const Vector& hingeAxis)
    {
        DeliveryBoxGeometry::OrientedBox collider;

        collider.center = Vector();

        collider.size = Vector();

        collider.right = Vector(1.0f, 0.0f, 0.0f);

        collider.up = Vector(0.0f, 1.0f, 0.0f);

        collider.forward = Vector(0.0f, 0.0f, 1.0f);

        if (vertices.size() < 2)
        {
            return collider;
        }

        Vector right = hingeAxis;

        if (right.lengthSquared() <= 0.000001f)
        {
            right = Vector(1.0f, 0.0f, 0.0f);
        }

        right.normalize();

        float shortestDistanceSquared = std::numeric_limits<float>::max();

        Vector shortestEdge;

        // Die kürzeste Verbindung zweier eindeutiger Eckpunkte liefert beim Deckel dessen Materialstärke
        for (std::size_t firstIndex = 0; firstIndex < vertices.size(); ++firstIndex)
        {
            for (std::size_t secondIndex = firstIndex + 1; secondIndex < vertices.size();
                 ++secondIndex)
            {
                Vector edge = vertices[secondIndex] - vertices[firstIndex];

                float distanceSquared = edge.lengthSquared();

                if (distanceSquared <= 0.00000001f)
                {
                    continue;
                }

                if (distanceSquared < shortestDistanceSquared)
                {
                    shortestDistanceSquared = distanceSquared;

                    shortestEdge = edge;
                }
            }
        }

        if (shortestEdge.lengthSquared() <= 0.000001f)
        {
            return collider;
        }

        // Beim Deckelmodell entspricht die kürzeste Kante der Materialstärke
        Vector up = shortestEdge - right * shortestEdge.dot(right);

        if (up.lengthSquared() <= 0.000001f)
        {
            return collider;
        }

        up.normalize();

        Vector forward = right.cross(up);

        if (forward.lengthSquared() <= 0.000001f)
        {
            return collider;
        }

        forward.normalize();

        // Richtet alle Achsen erneut exakt rechtwinklig zueinander aus
        up = forward.cross(right);

        up.normalize();

        // Alle Eckpunkte werden auf die drei lokalen Achsen projiziert, um Größe und Mittelpunkt der OBB zu bestimmen
        float minimumRight = vertices[0].dot(right);

        float maximumRight = minimumRight;

        float minimumUp = vertices[0].dot(up);

        float maximumUp = minimumUp;

        float minimumForward = vertices[0].dot(forward);

        float maximumForward = minimumForward;

        for (std::size_t index = 1; index < vertices.size(); ++index)
        {
            float rightProjection = vertices[index].dot(right);

            float upProjection = vertices[index].dot(up);

            float forwardProjection = vertices[index].dot(forward);

            minimumRight = std::min(minimumRight, rightProjection);

            maximumRight = std::max(maximumRight, rightProjection);

            minimumUp = std::min(minimumUp, upProjection);

            maximumUp = std::max(maximumUp, upProjection);

            minimumForward = std::min(minimumForward, forwardProjection);

            maximumForward = std::max(maximumForward, forwardProjection);
        }

        float centerRight = (minimumRight + maximumRight) * 0.5f;

        float centerUp = (minimumUp + maximumUp) * 0.5f;

        float centerForward = (minimumForward + maximumForward) * 0.5f;

        collider.center = right * centerRight + up * centerUp + forward * centerForward;

        collider.size = Vector(maximumRight - minimumRight,
                               maximumUp - minimumUp,
                               maximumForward - minimumForward);

        collider.right = right;

        collider.up = up;

        collider.forward = forward;

        return collider;
    }
}

DeliveryBoxGeometry createDeliveryBoxGeometry(const AABB& outerBounds,
                                              const AABB& lidBounds,
                                              const std::vector<Vector>& lidVertices,
                                              const DeliveryBoxGeometrySettings& settings)
{
    DeliveryBoxGeometry geometry;

    geometry.outerBounds = outerBounds;

    geometry.lidBounds = lidBounds;

    geometry.lidHingeAxis = Vector(1.0f, 0.0f, 0.0f);

    geometry.lidVertices = createUniqueVertices(lidVertices);

    geometry.lidPhysicsCollider =
        createLidPhysicsCollider(geometry.lidVertices, geometry.lidHingeAxis);

    Vector outerSize = outerBounds.size();

    float horizontalSize = std::min(outerSize.X, outerSize.Z);

    float wallThickness = horizontalSize * settings.wallThicknessRatio;

    float bottomThickness = outerSize.Y * settings.bottomThicknessRatio;

    // Der Innenraum wird aus Außenmaß, Wandstärke und Bodenstärke abgeleitet statt hart codiert
    geometry.innerBounds = AABB(Vector(outerBounds.Min.X + wallThickness,
                                       outerBounds.Min.Y + bottomThickness,
                                       outerBounds.Min.Z + wallThickness),
                                Vector(outerBounds.Max.X - wallThickness,
                                       outerBounds.Max.Y,
                                       outerBounds.Max.Z - wallThickness));

    Vector innerSize = geometry.innerBounds.size();

    float lidClearanceHeight = innerSize.Y * settings.lidClearanceHeightRatio;

    geometry.lidClearanceBounds = AABB(Vector(geometry.innerBounds.Min.X,
                                              geometry.innerBounds.Max.Y - lidClearanceHeight,
                                              geometry.innerBounds.Min.Z),
                                       geometry.innerBounds.Max);

    AABB bottomBounds(Vector(outerBounds.Min.X, outerBounds.Min.Y, outerBounds.Min.Z),
                      Vector(outerBounds.Max.X, geometry.innerBounds.Min.Y, outerBounds.Max.Z));

    AABB leftWallBounds(Vector(outerBounds.Min.X, geometry.innerBounds.Min.Y, outerBounds.Min.Z),
                        Vector(geometry.innerBounds.Min.X, outerBounds.Max.Y, outerBounds.Max.Z));

    AABB rightWallBounds(
        Vector(geometry.innerBounds.Max.X, geometry.innerBounds.Min.Y, outerBounds.Min.Z),
        Vector(outerBounds.Max.X, outerBounds.Max.Y, outerBounds.Max.Z));

    AABB backWallBounds(Vector(outerBounds.Min.X, geometry.innerBounds.Min.Y, outerBounds.Min.Z),
                        Vector(outerBounds.Max.X, outerBounds.Max.Y, geometry.innerBounds.Min.Z));

    AABB frontWallBounds(
        Vector(outerBounds.Min.X, geometry.innerBounds.Min.Y, geometry.innerBounds.Max.Z),
        Vector(outerBounds.Max.X, outerBounds.Max.Y, outerBounds.Max.Z));

    geometry.parts[0] = createPart(bottomBounds);

    geometry.parts[1] = createPart(leftWallBounds);

    geometry.parts[2] = createPart(rightWallBounds);

    geometry.parts[3] = createPart(backWallBounds);

    geometry.parts[4] = createPart(frontWallBounds);

    float centerX = outerBounds.center().X;

    geometry.lidHingePosition = Vector(centerX, outerBounds.Max.Y, outerBounds.Min.Z);

    return geometry;
}
