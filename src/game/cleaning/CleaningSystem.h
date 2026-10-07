#pragma once

#include "Spill.h"

#include <random>
#include <vector>

// Zustand des periodisch ausgeloesten Reinigungsereignisses
enum class CleaningState
{
    Waiting,
    Active
};

// Steuert Wartezeit und zufaelliges Erzeugen eines einzelnen aktiven Spills
class CleaningSystem
{
public:
    CleaningSystem();

    void initialize(Spill* spill, const std::vector<Vector>& spawnPoints);

    void update(float deltaTime);

    bool isSpillActive() const;

    bool consumeSpillStartedEvent();

private:
    void startWaiting();

    bool spawnSpill();

    float randomWaitingTime();

    int randomSpawnPointIndex();

    Spill* m_spill;

    std::vector<Vector> m_spawnPoints;

    CleaningState m_state;

    std::mt19937 m_randomGenerator;

    float m_waitingTimeRemaining;

    int m_lastSpawnPointIndex;

    bool m_initialized;

    bool m_spillStartedEvent;
};
