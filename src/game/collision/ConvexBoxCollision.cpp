
#include "ConvexBoxCollision.h"

#include <cstddef>

bool ConvexBoxCollision::overlaps(const std::vector<Vector>& firstVertices,
                                  const std::vector<Vector>& secondVertices,
                                  float contactTolerance)
{
    if (firstVertices.size() < 8 || secondVertices.size() < 8)
    {
        return false;
    }

    std::vector<Vector> firstDirections = createVertexDirections(firstVertices);

    std::vector<Vector> secondDirections = createVertexDirections(secondVertices);

    // Kreuzprodukte zweier Kanten derselben Box liefern deren mögliche Flächennormalen
    for (std::size_t firstIndex = 0; firstIndex < firstDirections.size(); ++firstIndex)
    {
        for (std::size_t secondIndex = firstIndex + 1; secondIndex < firstDirections.size();
             ++secondIndex)
        {
            Vector axis = firstDirections[firstIndex].cross(firstDirections[secondIndex]);

            if (!overlapsOnAxis(firstVertices, secondVertices, axis, contactTolerance))
            {
                return false;
            }
        }
    }

    for (std::size_t firstIndex = 0; firstIndex < secondDirections.size(); ++firstIndex)
    {
        for (std::size_t secondIndex = firstIndex + 1; secondIndex < secondDirections.size();
             ++secondIndex)
        {
            Vector axis = secondDirections[firstIndex].cross(secondDirections[secondIndex]);

            if (!overlapsOnAxis(firstVertices, secondVertices, axis, contactTolerance))
            {
                return false;
            }
        }
    }

    // Bei zwei gedrehten Boxen entstehen weitere Trennachsen aus den Kanten beider Boxen
    for (const Vector& firstDirection : firstDirections)
    {
        for (const Vector& secondDirection : secondDirections)
        {
            Vector axis = firstDirection.cross(secondDirection);

            if (!overlapsOnAxis(firstVertices, secondVertices, axis, contactTolerance))
            {
                return false;
            }
        }
    }

    return true;
}

std::vector<Vector> ConvexBoxCollision::transformVertices(const std::vector<Vector>& localVertices,
                                                          const Matrix& worldTransform)
{
    std::vector<Vector> worldVertices;

    worldVertices.reserve(localVertices.size());

    for (const Vector& localVertex : localVertices)
    {
        worldVertices.push_back(worldTransform * localVertex);
    }

    return worldVertices;
}

std::vector<Vector> ConvexBoxCollision::createVertexDirections(const std::vector<Vector>& vertices)
{
    std::vector<Vector> directions;

    for (std::size_t firstIndex = 0; firstIndex < vertices.size(); ++firstIndex)
    {
        for (std::size_t secondIndex = firstIndex + 1; secondIndex < vertices.size(); ++secondIndex)
        {
            Vector direction = vertices[secondIndex] - vertices[firstIndex];

            if (direction.lengthSquared() <= 0.000001f)
            {
                continue;
            }

            directions.push_back(direction);
        }
    }

    return directions;
}

bool ConvexBoxCollision::overlapsOnAxis(const std::vector<Vector>& firstVertices,
                                        const std::vector<Vector>& secondVertices,
                                        Vector axis,
                                        float contactTolerance)
{
    if (axis.lengthSquared() <= 0.000001f)
    {
        return true;
    }

    // Für die Separating-Axis-Prüfung werden beide Punktmengen auf dieselbe normalisierte Achse projiziert
    axis.normalize();

    float firstMinimum = firstVertices.front().dot(axis);

    float firstMaximum = firstMinimum;

    for (std::size_t index = 1; index < firstVertices.size(); ++index)
    {
        float projection = firstVertices[index].dot(axis);

        if (projection < firstMinimum)
        {
            firstMinimum = projection;
        }

        if (projection > firstMaximum)
        {
            firstMaximum = projection;
        }
    }

    float secondMinimum = secondVertices.front().dot(axis);

    float secondMaximum = secondMinimum;

    for (std::size_t index = 1; index < secondVertices.size(); ++index)
    {
        float projection = secondVertices[index].dot(axis);

        if (projection < secondMinimum)
        {
            secondMinimum = projection;
        }

        if (projection > secondMaximum)
        {
            secondMaximum = projection;
        }
    }

    return firstMaximum + contactTolerance >= secondMinimum
           && secondMaximum + contactTolerance >= firstMinimum;
}
