//
//  Aabb.hpp
//  CGXcode
//
//  Created by Philipp Lensing on 02.11.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#ifndef Aabb_hpp
#define Aabb_hpp

#include "Matrix.h"
#include "vector.h"

class AABB
{
public:
    Vector Min;
    Vector Max;

    AABB();
    AABB(const Vector& min, const Vector& max);
    AABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);

    static AABB fromCenterAndSize(const Vector& center, const Vector& size);

    Vector size() const;
    Vector center() const;

    bool contains(const Vector& point) const;
    bool overlaps(const AABB& other) const;

    AABB transform(const Matrix& matrix) const;
    AABB merge(const AABB& first, const AABB& second) const;

    AABB& merge(const AABB& other);

    void corners(Vector result[8]) const;

    void fromPoints(const Vector* points, unsigned int pointCount);

    static const AABB& unitBox();

protected:
    static AABB s_unitBox;
};

#endif /* Aabb_hpp */
