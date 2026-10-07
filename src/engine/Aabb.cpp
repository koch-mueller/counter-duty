//
//  Aabb.cpp
//  CGXcode
//
//  Created by Philipp Lensing on 02.11.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#include "Aabb.h"

AABB AABB::s_unitBox(Vector(-1.0f, -1.0f, -1.0f), Vector(1.0f, 1.0f, 1.0f));

AABB::AABB()
{
}

AABB::AABB(const Vector& min, const Vector& max)
    : Min(min),
      Max(max)
{
}

AABB::AABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
    : Min(minX, minY, minZ),
      Max(maxX, maxY, maxZ)
{
}

AABB AABB::fromCenterAndSize(const Vector& center, const Vector& size)
{
    Vector halfSize = size * 0.5f;

    return AABB(center - halfSize, center + halfSize);
}

Vector AABB::size() const
{
    return Max - Min;
}

Vector AABB::center() const
{
    return (Max + Min) * 0.5f;
}

bool AABB::contains(const Vector& point) const
{
    return point.X >= Min.X && point.X <= Max.X && point.Y >= Min.Y && point.Y <= Max.Y
           && point.Z >= Min.Z && point.Z <= Max.Z;
}

bool AABB::overlaps(const AABB& other) const
{
    return !(Max.X < other.Min.X || Min.X > other.Max.X || Max.Y < other.Min.Y
             || Min.Y > other.Max.Y || Max.Z < other.Min.Z || Min.Z > other.Max.Z);
}

AABB AABB::transform(const Matrix& matrix) const
{
    Vector transformedCorners[8];

    corners(transformedCorners);

    for (int index = 0; index < 8; ++index)
    {
        transformedCorners[index] = matrix * transformedCorners[index];
    }

    AABB result;

    result.fromPoints(transformedCorners, 8);

    return result;
}

AABB AABB::merge(const AABB& first, const AABB& second) const
{
    AABB result;

    result.Min.X = first.Min.X < second.Min.X ? first.Min.X : second.Min.X;

    result.Min.Y = first.Min.Y < second.Min.Y ? first.Min.Y : second.Min.Y;

    result.Min.Z = first.Min.Z < second.Min.Z ? first.Min.Z : second.Min.Z;

    result.Max.X = first.Max.X > second.Max.X ? first.Max.X : second.Max.X;

    result.Max.Y = first.Max.Y > second.Max.Y ? first.Max.Y : second.Max.Y;

    result.Max.Z = first.Max.Z > second.Max.Z ? first.Max.Z : second.Max.Z;

    return result;
}

AABB& AABB::merge(const AABB& other)
{
    Min.X = other.Min.X < Min.X ? other.Min.X : Min.X;

    Min.Y = other.Min.Y < Min.Y ? other.Min.Y : Min.Y;

    Min.Z = other.Min.Z < Min.Z ? other.Min.Z : Min.Z;

    Max.X = other.Max.X > Max.X ? other.Max.X : Max.X;

    Max.Y = other.Max.Y > Max.Y ? other.Max.Y : Max.Y;

    Max.Z = other.Max.Z > Max.Z ? other.Max.Z : Max.Z;

    return *this;
}

void AABB::corners(Vector result[8]) const
{
    result[0] = Vector(Min.X, Min.Y, Min.Z);

    result[1] = Vector(Max.X, Min.Y, Min.Z);

    result[2] = Vector(Max.X, Max.Y, Min.Z);

    result[3] = Vector(Min.X, Max.Y, Min.Z);

    result[4] = Vector(Min.X, Min.Y, Max.Z);

    result[5] = Vector(Max.X, Min.Y, Max.Z);

    result[6] = Vector(Max.X, Max.Y, Max.Z);

    result[7] = Vector(Min.X, Max.Y, Max.Z);
}

void AABB::fromPoints(const Vector* points, unsigned int pointCount)
{
    Max = Vector(-1e20f, -1e20f, -1e20f);

    Min = Vector(1e20f, 1e20f, 1e20f);

    for (unsigned int index = 0; index < pointCount; ++index)
    {
        if (Min.X > points[index].X)
        {
            Min.X = points[index].X;
        }

        if (Min.Y > points[index].Y)
        {
            Min.Y = points[index].Y;
        }

        if (Min.Z > points[index].Z)
        {
            Min.Z = points[index].Z;
        }

        if (Max.X < points[index].X)
        {
            Max.X = points[index].X;
        }

        if (Max.Y < points[index].Y)
        {
            Max.Y = points[index].Y;
        }

        if (Max.Z < points[index].Z)
        {
            Max.Z = points[index].Z;
        }
    }
}

const AABB& AABB::unitBox()
{
    return s_unitBox;
}
