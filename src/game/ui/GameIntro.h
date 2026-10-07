#pragma once

// Zeitgesteuerter Ablauf der Intro-Einblendungen vor dem Hauptmenü
class GameIntro
{
public:
    GameIntro();

    void update(float deltaTime);
    void reset();

    float studioBrightness() const;
    float titleBrightness() const;

    bool consumeSoundRequest();

    bool canStartLoading() const;
    void notifyLoadingComplete();

    bool isFinished() const;

private:
    float fadeOutStart() const;
    float globalFadeOut() const;

    float m_elapsedTime;
    float m_loadingCompleteTime;

    bool m_soundRequested;
    bool m_soundPlayed;
    bool m_loadingComplete;
};
