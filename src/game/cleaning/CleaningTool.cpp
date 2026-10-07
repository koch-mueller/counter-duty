#include "CleaningTool.h"

namespace
{
    constexpr float kCameraMargin = 0.08f;
}

CleaningTool::CleaningTool()
    : m_size(0.1f, 1.0f, 0.1f),
      m_worldTransform(),
      m_collider(),
      m_interactable(true),
      m_minimumCarryDistance(0.1f),
      m_carried(false)
{
}

void CleaningTool::configure(const Vector& position, const Vector& size)
{
    m_size = size;

    float halfWidth = m_size.X * 0.5f;

    float halfDepth = m_size.Z * 0.5f;

    // Der gesamte Mop-Collider soll beim Tragen vor der Kamera bleiben
    m_minimumCarryDistance =
        std::sqrt(halfWidth * halfWidth + halfDepth * halfDepth) + kCameraMargin;

    m_worldTransform.translation(position);

    m_interactable = true;

    updateCollider();
}

bool CleaningTool::canInteract() const
{
    return m_interactable;
}

std::string CleaningTool::getPromptText() const
{
    return "[E] Wischmop aufnehmen";
}

void CleaningTool::interact()
{
}

const AABB& CleaningTool::getCollider() const
{
    return m_collider;
}

bool CleaningTool::canBeCarried() const
{
    return true;
}

Matrix CleaningTool::getWorldTransform() const
{
    return m_worldTransform;
}

void CleaningTool::setWorldTransform(const Matrix& transform)
{
    m_worldTransform = transform;

    updateCollider();
}

AABB CleaningTool::getLocalBounds() const
{
    Vector halfSize = m_size * 0.5f;

    return AABB(-halfSize, halfSize);
}

void CleaningTool::onCarryStarted()
{
    m_interactable = false;

    m_carried = true;
}

void CleaningTool::onCarryEnded()
{
    Vector position = m_worldTransform.translation();

    // Beim Ablegen wird der Mop wieder auf Bodenhoehe gesetzt
    position.Y = m_size.Y * 0.5f;

    m_worldTransform.translation(position);

    m_interactable = true;

    m_carried = false;

    updateCollider();
}

float CleaningTool::carryDistance() const
{
    return 1.9f;
}

float CleaningTool::minimumCarryDistance() const
{
    return m_minimumCarryDistance;
}

float CleaningTool::carryVerticalOffset() const
{
    return -1.0f;
}

bool CleaningTool::allowsCarryRotation() const
{
    return false;
}

bool CleaningTool::usesCameraRotation() const
{
    return false;
}

void CleaningTool::updateCollider()
{
    m_collider = getLocalBounds().transform(m_worldTransform);
}

bool CleaningTool::usesCameraPitchForCarryPosition() const
{
    return false;
}

Vector CleaningTool::cleaningPosition() const
{
    // Gereinigt wird am unteren Mittelpunkt des Mops und nicht am Objektzentrum
    Vector localCleaningPosition(0.0f, -m_size.Y * 0.5f, 0.0f);

    return m_worldTransform * localCleaningPosition;
}

bool CleaningTool::isCarried() const
{
    return m_carried;
}
