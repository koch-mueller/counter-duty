#include "InteractionSystem.h"

#include "../collision/Ray.h"
#include "../collision/Raycast.h"

#include <algorithm>
#include <limits>

InteractionSystem::InteractionSystem(float maxInteractionDistance)
    : m_focusedInteractable(nullptr),
      m_maxInteractionDistance(maxInteractionDistance),
      m_blockingColliders(nullptr)
{
}

void InteractionSystem::addInteractable(IInteractable* interactable)
{
    if (interactable == nullptr)
    {
        return;
    }

    auto existingInteractable =
        std::find(m_interactables.begin(), m_interactables.end(), interactable);

    if (existingInteractable != m_interactables.end())
    {
        return;
    }

    m_interactables.push_back(interactable);
}

void InteractionSystem::removeInteractable(IInteractable* interactable)
{
    if (interactable == nullptr)
    {
        return;
    }

    m_interactables.remove(interactable);

    if (m_focusedInteractable == interactable)
    {
        m_focusedInteractable = nullptr;
    }
}

void InteractionSystem::clear()
{
    m_interactables.clear();

    m_focusedInteractable = nullptr;
}

void InteractionSystem::update(const FirstPersonCamera& camera)
{
    m_focusedInteractable = nullptr;

    Ray currentRay;

    currentRay.origin = camera.position();

    currentRay.direction = camera.frontVector();

    // Zuerst wird die nächste Wand/Barriere gesucht. Interaktionsobjekte dahinter dürfen nicht durch Hindernisse angeklickt werden
    float nearestBlockingDistance = std::numeric_limits<float>::infinity();

    if (m_blockingColliders != nullptr)
    {
        for (const AABB& blockingCollider : *m_blockingColliders)
        {
            float hitDistance = -1.0f;

            if (!Raycast::rayIntersectsAABB(currentRay, blockingCollider, hitDistance))
            {
                continue;
            }

            if (hitDistance <= m_maxInteractionDistance && hitDistance < nearestBlockingDistance)
            {
                nearestBlockingDistance = hitDistance;
            }
        }
    }

    float nearestHitDistance = m_maxInteractionDistance;

    int focusedPriority = std::numeric_limits<int>::min();

    // Eine kleine Toleranz erlaubt Interaktionen mit Objekten, deren Collider direkt auf einer blockierenden Fläche liegt
    const float blockingContactTolerance = 0.001f;

    for (IInteractable* interactable : m_interactables)
    {
        if (interactable == nullptr || !interactable->canInteract())
        {
            continue;
        }

        std::vector<AABB> colliders = interactable->getInteractionColliders();

        bool wasHit = false;

        float nearestInteractableDistance = m_maxInteractionDistance;

        for (const AABB& collider : colliders)
        {
            float hitDistance = -1.0f;

            if (!Raycast::rayIntersectsAABB(currentRay, collider, hitDistance))
            {
                continue;
            }

            bool isInRange = hitDistance <= m_maxInteractionDistance;

            bool isNotBehindBlocker =
                hitDistance <= nearestBlockingDistance + blockingContactTolerance;

            bool isCloser = hitDistance < nearestInteractableDistance;

            if (isInRange && isNotBehindBlocker && isCloser)
            {
                nearestInteractableDistance = hitDistance;

                wasHit = true;
            }
        }

        if (!wasHit)
        {
            continue;
        }

        // Höhere Priorität gewinnt unabhängig von kleinen Distanzunterschieden; bei gleicher Priorität gewinnt das nähere Objekt
        int priority = interactable->interactionPriority();

        bool hasHigherPriority = priority > focusedPriority;

        bool hasSamePriorityAndIsCloser =
            priority == focusedPriority && nearestInteractableDistance < nearestHitDistance;

        if (!hasHigherPriority && !hasSamePriorityAndIsCloser)
        {
            continue;
        }

        focusedPriority = priority;

        nearestHitDistance = nearestInteractableDistance;

        m_focusedInteractable = interactable;
    }
}

void InteractionSystem::interact()
{
    if (m_focusedInteractable == nullptr || !m_focusedInteractable->canInteract())
    {
        return;
    }

    m_focusedInteractable->interact();
}

void InteractionSystem::secondaryInteract()
{
    if (m_focusedInteractable == nullptr || !m_focusedInteractable->canInteract())
    {
        return;
    }

    m_focusedInteractable->secondaryInteract();
}

void InteractionSystem::setBlockingColliders(const std::list<AABB>& colliders)
{
    m_blockingColliders = &colliders;
}

IInteractable* InteractionSystem::focusedInteractable() const
{
    return m_focusedInteractable;
}

const std::list<IInteractable*>& InteractionSystem::interactables() const
{
    return m_interactables;
}

std::string InteractionSystem::interactionPrompt() const
{
    if (m_focusedInteractable == nullptr)
    {
        return "";
    }

    return m_focusedInteractable->getPromptText();
}
