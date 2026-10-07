
#include "GameIntro.h"

#include <algorithm>

namespace
{
    // Kurzer schwarzer Bildschirm am Anfang
    constexpr float kSoundTime = 1.65f;

    // KOCH STUDIOS
    constexpr float kStudioFadeInStart = 1.55f;

    constexpr float kStudioFadeInEnd = 2.55f;

    // Kurze Pause, bevor der Spieltitel erscheint
    constexpr float kTitleFadeInStart = 3.00f;

    constexpr float kTitleFadeInEnd = 4.20f;

    // Selbst wenn das Laden sehr schnell fertig ist, bleibt das komplette Intro noch etwas sichtbar
    constexpr float kEarliestFadeOutStart = 4.40f;

    // Wenn das Laden länger dauert, bleibt der fertige Titel nach dem Laden noch kurz stehen
    constexpr float kPostLoadingHoldTime = 0.65f;

    // Beide Texte werden langsam gemeinsam ausgeblendet
    constexpr float kFadeOutDuration = 1.20f;

    constexpr float kMaximumDeltaTime = 0.1f;
}

GameIntro::GameIntro()
    : m_elapsedTime(0.0f),
      m_loadingCompleteTime(0.0f),
      m_soundRequested(false),
      m_soundPlayed(false),
      m_loadingComplete(false)
{
}

// Die Intro-Zeit wird begrenzt weitergeschaltet, damit einzelne lange Frames keine Fade-Phase überspringen
void GameIntro::update(float deltaTime)
{
    if (deltaTime <= 0.0f)
    {
        return;
    }

    deltaTime = std::min(deltaTime, kMaximumDeltaTime);

    m_elapsedTime += deltaTime;

    if (!m_soundRequested && m_elapsedTime >= kSoundTime)
    {
        m_soundRequested = true;
    }
}

void GameIntro::reset()
{
    m_elapsedTime = 0.0f;

    m_loadingCompleteTime = 0.0f;

    m_soundRequested = false;

    m_soundPlayed = false;

    m_loadingComplete = false;
}

float GameIntro::studioBrightness() const
{
    float brightness = 0.0f;

    if (m_elapsedTime < kStudioFadeInStart)
    {
        brightness = 0.0f;
    }
    else if (m_elapsedTime < kStudioFadeInEnd)
    {
        brightness = (m_elapsedTime - kStudioFadeInStart) / (kStudioFadeInEnd - kStudioFadeInStart);
    }
    else
    {
        brightness = 1.0f;
    }

    return brightness * globalFadeOut();
}

float GameIntro::titleBrightness() const
{
    float brightness = 0.0f;

    if (m_elapsedTime < kTitleFadeInStart)
    {
        brightness = 0.0f;
    }
    else if (m_elapsedTime < kTitleFadeInEnd)
    {
        brightness = (m_elapsedTime - kTitleFadeInStart) / (kTitleFadeInEnd - kTitleFadeInStart);
    }
    else
    {
        brightness = 1.0f;
    }

    return brightness * globalFadeOut();
}

bool GameIntro::consumeSoundRequest()
{
    if (!m_soundRequested || m_soundPlayed)
    {
        return false;
    }

    m_soundPlayed = true;

    return true;
}

bool GameIntro::canStartLoading() const
{
    // Erst laden, wenn beide Texte komplett eingeblendet wurden
    return m_elapsedTime >= kTitleFadeInEnd;
}

void GameIntro::notifyLoadingComplete()
{
    if (m_loadingComplete)
    {
        return;
    }

    m_loadingComplete = true;

    m_loadingCompleteTime = m_elapsedTime;
}

bool GameIntro::isFinished() const
{
    if (!m_loadingComplete)
    {
        return false;
    }

    return m_elapsedTime >= fadeOutStart() + kFadeOutDuration;
}

float GameIntro::fadeOutStart() const
{
    return std::max(kEarliestFadeOutStart, m_loadingCompleteTime + kPostLoadingHoldTime);
}

// Der gemeinsame Faktor blendet Studio- und Titeltext synchron aus, sobald das Laden abgeschlossen ist
float GameIntro::globalFadeOut() const
{
    if (!m_loadingComplete)
    {
        return 1.0f;
    }

    float start = fadeOutStart();

    if (m_elapsedTime < start)
    {
        return 1.0f;
    }

    float end = start + kFadeOutDuration;

    if (m_elapsedTime >= end)
    {
        return 0.0f;
    }

    return 1.0f - (m_elapsedTime - start) / kFadeOutDuration;
}
