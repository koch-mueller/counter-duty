#pragma once

#include "../../engine/Aabb.h"
#include "../../engine/Matrix.h"

// Gemeinsame Schnittstelle für Objekte, die vom CarrySystem aufgenommen werden können
class ICarryable
{
public:
    virtual ~ICarryable() = default;

    virtual bool canBeCarried() const = 0;

    virtual Matrix getWorldTransform() const = 0;

    virtual void setWorldTransform(const Matrix& transform) = 0;

    virtual AABB getLocalBounds() const = 0;

    virtual void onCarryStarted() = 0;
    virtual void onCarryEnded() = 0;

    virtual float carryDistance() const
    {
        return 1.2f;
    }

    virtual float minimumCarryDistance() const
    {
        return 0.1f;
    }

    virtual float carryVerticalOffset() const
    {
        return 0.0f;
    }

    virtual bool allowsCarryRotation() const
    {
        return true;
    }

    virtual bool usesDynamicBlockingColliders() const
    {
        return true;
    }

    virtual bool usesCameraRotation() const
    {
        return true;
    }

    virtual bool requiresPlayerClearanceOnDrop() const
    {
        return false;
    }

    virtual bool usesCameraPitchForCarryPosition() const
    {
        return true;
    }
};
