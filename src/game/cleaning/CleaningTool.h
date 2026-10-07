#pragma once

#include "../carry/ICarryable.h"
#include "../interaction/IInteractable.h"

// Repräsentiert den aufnehmbaren Wischmop inklusive Collider und Reinigungspunkt
class CleaningTool : public IInteractable, public ICarryable
{
public:
    CleaningTool();

    void configure(const Vector& position, const Vector& size);

    bool canInteract() const override;

    std::string getPromptText() const override;

    void interact() override;

    const AABB& getCollider() const override;

    bool canBeCarried() const override;

    Matrix getWorldTransform() const override;

    void setWorldTransform(const Matrix& transform) override;

    AABB getLocalBounds() const override;

    void onCarryStarted() override;

    void onCarryEnded() override;

    float carryDistance() const override;

    float carryVerticalOffset() const override;

    bool allowsCarryRotation() const override;

    bool usesCameraRotation() const override;

    float minimumCarryDistance() const override;

    bool usesCameraPitchForCarryPosition() const override;

    Vector cleaningPosition() const;

    bool isCarried() const;

private:
    void updateCollider();

    Vector m_size;

    Matrix m_worldTransform;

    AABB m_collider;

    bool m_interactable;

    float m_minimumCarryDistance;

    bool m_carried;
};
