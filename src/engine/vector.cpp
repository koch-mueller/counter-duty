#include "vector.h"

#include <cmath>

namespace
{
    constexpr float kEpsilon = 0.000001f;
}

Vector::Vector()
    : X(0.0f),
      Y(0.0f),
      Z(0.0f)
{
}

Vector::Vector(float x, float y, float z)
    : X(x),
      Y(y),
      Z(z)
{
}

float Vector::dot(const Vector& vector) const
{
    return X * vector.X + Y * vector.Y + Z * vector.Z;
}

Vector Vector::cross(const Vector& vector) const
{
    return Vector(Y * vector.Z - Z * vector.Y,
                  Z * vector.X - X * vector.Z,
                  X * vector.Y - Y * vector.X);
}

Vector Vector::operator+(const Vector& vector) const
{
    return Vector(X + vector.X, Y + vector.Y, Z + vector.Z);
}

Vector Vector::operator-(const Vector& vector) const
{
    return Vector(X - vector.X, Y - vector.Y, Z - vector.Z);
}

Vector Vector::operator*(float factor) const
{
    return Vector(X * factor, Y * factor, Z * factor);
}

Vector Vector::operator-() const
{
    return Vector(-X, -Y, -Z);
}

Vector& Vector::operator+=(const Vector& vector)
{
    X += vector.X;
    Y += vector.Y;
    Z += vector.Z;

    return *this;
}

Vector& Vector::normalize()
{
    float currentLength = length();

    if (currentLength <= kEpsilon)
    {
        X = 0.0f;
        Y = 0.0f;
        Z = 0.0f;

        return *this;
    }

    float inverseLength = 1.0f / currentLength;

    X *= inverseLength;
    Y *= inverseLength;
    Z *= inverseLength;

    return *this;
}

float Vector::length() const
{
    return std::sqrt(lengthSquared());
}

float Vector::lengthSquared() const
{
    return X * X + Y * Y + Z * Z;
}

Vector Vector::reflection(const Vector& normal) const
{
    float normalLengthSquared = normal.lengthSquared();

    if (normalLengthSquared <= kEpsilon)
    {
        return *this;
    }

    float reflectionFactor = 2.0f * dot(normal) / normalLengthSquared;

    return *this - normal * reflectionFactor;
}

bool Vector::triangleIntersection(const Vector& direction,
                                  const Vector& firstVertex,
                                  const Vector& secondVertex,
                                  const Vector& thirdVertex,
                                  float& distance) const
{
    Vector firstEdge = secondVertex - firstVertex;

    Vector secondEdge = thirdVertex - firstVertex;

    Vector directionCrossSecondEdge = direction.cross(secondEdge);

    float determinant = firstEdge.dot(directionCrossSecondEdge);

    if (std::fabs(determinant) <= kEpsilon)
    {
        return false;
    }

    float inverseDeterminant = 1.0f / determinant;

    Vector originOffset = *this - firstVertex;

    float firstTriangleCoordinate = originOffset.dot(directionCrossSecondEdge) * inverseDeterminant;

    if (firstTriangleCoordinate < 0.0f || firstTriangleCoordinate > 1.0f)
    {
        return false;
    }

    Vector originCrossFirstEdge = originOffset.cross(firstEdge);

    float secondTriangleCoordinate = direction.dot(originCrossFirstEdge) * inverseDeterminant;

    if (secondTriangleCoordinate < 0.0f
        || firstTriangleCoordinate + secondTriangleCoordinate > 1.0f)
    {
        return false;
    }

    float intersectionDistance = secondEdge.dot(originCrossFirstEdge) * inverseDeterminant;

    if (intersectionDistance < 0.0f)
    {
        return false;
    }

    distance = intersectionDistance;

    return true;
}
