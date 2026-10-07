#pragma once

#include "../carry/ICarryable.h"
#include "../interaction/IInteractable.h"
#include "DeliveryBox.h"

#include <string>
#include <vector>

// Verbindet den Lieferkarton mit Interaktions- und Carry-System
class DeliveryBoxInteraction : public IInteractable, public ICarryable
{
public:
    explicit DeliveryBoxInteraction(DeliveryBox* deliveryBox);

    bool canInteract() const override;
    std::string getPromptText() const override;
    void interact() override;
    bool secondaryInteract() override;

    const AABB& getCollider() const override;
    std::vector<AABB> getInteractionColliders() const override;
    int interactionPriority() const override;

    void updateLidColliders(const std::vector<Vector>& backLidWorldVertices,
                            const std::vector<Vector>& frontLidWorldVertices);

    bool hasLidColliders() const;
    const AABB& backLidCollider() const;
    const AABB& frontLidCollider() const;

    bool canBeCarried() const override;

    Matrix getWorldTransform() const override;
    void setWorldTransform(const Matrix& transform) override;

    AABB getLocalBounds() const override;

    void onCarryStarted() override;
    void onCarryEnded() override;

    float carryDistance() const override;
    float minimumCarryDistance() const override;
    float carryVerticalOffset() const override;

    bool allowsCarryRotation() const override;
    bool usesDynamicBlockingColliders() const override;
    bool usesCameraRotation() const override;
    bool requiresPlayerClearanceOnDrop() const override;

private:
    void updateCombinedCollider();

    DeliveryBox* m_deliveryBox;

    AABB m_backLidCollider;
    AABB m_frontLidCollider;
    AABB m_combinedCollider;

    bool m_hasLidColliders;
};
