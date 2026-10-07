#pragma once

#include "../../engine/Matrix.h"
#include "../../engine/vector.h"

#include <vector>

// Hilfsfunktionen für Überlappungstests zweier konvexer Boxen anhand ihrer Eckpunkte
class ConvexBoxCollision
{
public:
    static bool overlaps(const std::vector<Vector>& firstVertices,
                         const std::vector<Vector>& secondVertices,
                         float contactTolerance = 0.0005f);

    static std::vector<Vector> transformVertices(const std::vector<Vector>& localVertices,
                                                 const Matrix& worldTransform);

private:
    static std::vector<Vector> createVertexDirections(const std::vector<Vector>& vertices);

    static bool overlapsOnAxis(const std::vector<Vector>& firstVertices,
                               const std::vector<Vector>& secondVertices,
                               Vector axis,
                               float contactTolerance);
};
