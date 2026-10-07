#pragma once

class Vector
{
public:
    float X;
    float Y;
    float Z;

    Vector();

    Vector(float x, float y, float z);

    float dot(const Vector& vector) const;

    Vector cross(const Vector& vector) const;

    Vector operator+(const Vector& vector) const;

    Vector operator-(const Vector& vector) const;

    Vector operator*(float factor) const;

    Vector operator-() const;

    Vector& operator+=(const Vector& vector);

    Vector& normalize();

    float length() const;
    float lengthSquared() const;

    Vector reflection(const Vector& normal) const;

    bool triangleIntersection(const Vector& direction,
                              const Vector& firstVertex,
                              const Vector& secondVertex,
                              const Vector& thirdVertex,
                              float& distance) const;
};
