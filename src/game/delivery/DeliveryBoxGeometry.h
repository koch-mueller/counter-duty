#pragma once

#include "../../engine/Aabb.h"
#include "../../engine/vector.h"

#include <array>
#include <vector>

// Parameter, aus denen die Kollisions- und Innengeometrie des Kartons abgeleitet wird
struct DeliveryBoxGeometrySettings
{
    float wallThicknessRatio;
    float bottomThicknessRatio;
    float lidClearanceHeightRatio;
};

// Abgeleitete lokale Geometrie für Kartonwände, Innenraum und Deckel
struct DeliveryBoxGeometry
{
    struct Part
    {
        Vector center;
        Vector size;
    };

// Orientierte Box fuer Kollisions- und Deckelpruefungen, bestehend aus Transform und Groesse
struct OrientedBox
    {
        Vector center;
        Vector size;

        Vector right;
        Vector up;
        Vector forward;
    };

    AABB outerBounds;
    AABB innerBounds;
    AABB lidClearanceBounds;
    AABB lidBounds;

    std::vector<Vector> lidVertices;

    OrientedBox lidPhysicsCollider;

    std::array<Part, 5> parts;

    Vector lidHingePosition;
    Vector lidHingeAxis;
};

DeliveryBoxGeometry createDeliveryBoxGeometry(const AABB& outerBounds,
                                              const AABB& lidBounds,
                                              const std::vector<Vector>& lidVertices,
                                              const DeliveryBoxGeometrySettings& settings);
