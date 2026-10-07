
#include "CleaningSystem.h"

namespace
{
    constexpr float kMinimumWaitingTime = 45.0f;

    constexpr float kMaximumWaitingTime = 75.0f;
}

CleaningSystem::CleaningSystem()
    : m_spill(nullptr),
      m_spawnPoints(),
      m_state(CleaningState::Waiting),
      m_randomGenerator(std::random_device{}()),
      m_waitingTimeRemaining(0.0f),
      m_lastSpawnPointIndex(-1),
      m_initialized(false),
      m_spillStartedEvent(false)
{
}

void CleaningSystem::initialize(
    Spill* spill,
    const std::vector<Vector>& spawnPoints)
{
    m_spill = spill;

    m_spawnPoints = spawnPoints;

    m_lastSpawnPointIndex = -1;

    m_spillStartedEvent = false;

    m_initialized =
        m_spill != nullptr
        && !m_spawnPoints.empty();

    if (!m_initialized)
    {
        return;
    }

    m_spill->deactivate();

    startWaiting();
}

void CleaningSystem::update(float deltaTime)
{
    if (!m_initialized
        || m_spill == nullptr
        || deltaTime <= 0.0f)
    {
        return;
    }

    // Im Waiting-Zustand läuft nur der zufällige Timer herunter. Es kann nie mehr als ein Spill aktiv sein
    if (m_state == CleaningState::Waiting)
    {
        m_waitingTimeRemaining -= deltaTime;

        if (m_waitingTimeRemaining <= 0.0f)
        {
            spawnSpill();
        }

        return;
    }

    // Der Spill deaktiviert sich selbst nach vollständiger Reinigung; danach startet der nächste Wartezyklus
    if (m_state == CleaningState::Active
        && !m_spill->isActive())
    {
        startWaiting();
    }
}

bool CleaningSystem::consumeSpillStartedEvent()
{
    bool occurred = m_spillStartedEvent;

    m_spillStartedEvent = false;

    return occurred;
}

void CleaningSystem::startWaiting()
{
    m_state = CleaningState::Waiting;

    m_waitingTimeRemaining =
        randomWaitingTime();
}

bool CleaningSystem::spawnSpill()
{
    // Es existiert immer nur ein Spill. Bei mehreren Spawnpunkten wird der zuletzt verwendete Punkt vermieden
    if (m_spill == nullptr
        || m_spawnPoints.empty()
        || m_spill->isActive())
    {
        return false;
    }

    int spawnPointIndex =
        randomSpawnPointIndex();

    if (spawnPointIndex < 0)
    {
        return false;
    }

    m_spill->activate(
        m_spawnPoints[spawnPointIndex]);

    m_lastSpawnPointIndex =
        spawnPointIndex;

    m_state =
        CleaningState::Active;

    m_waitingTimeRemaining =
        0.0f;

    m_spillStartedEvent =
        true;

    return true;
}

float CleaningSystem::randomWaitingTime()
{
    std::uniform_real_distribution<float> distribution(
        kMinimumWaitingTime,
        kMaximumWaitingTime
    );

    return distribution(
        m_randomGenerator
    );
}

int CleaningSystem::randomSpawnPointIndex()
{
    if (m_spawnPoints.empty())
    {
        return -1;
    }

    if (m_spawnPoints.size() == 1)
    {
        return 0;
    }

    std::uniform_int_distribution<int> distribution(
        0,
        static_cast<int>(m_spawnPoints.size()) - 1
    );

    int spawnPointIndex =
        distribution(m_randomGenerator);

    // Bei mehreren Punkten wird derselbe Gang nicht direkt zweimal hintereinander gewählt
    while (spawnPointIndex
           == m_lastSpawnPointIndex)
    {
        spawnPointIndex =
            distribution(m_randomGenerator);
    }

    return spawnPointIndex;
}

bool CleaningSystem::isSpillActive() const
{
    return m_initialized
           && m_spill != nullptr
           && m_spill->isActive();
}
