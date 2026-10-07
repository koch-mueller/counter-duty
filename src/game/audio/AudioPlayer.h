#pragma once

#include "../../external/miniaudio/miniaudio.h"

#include <string>

// Verwaltet Hintergrundmusik und kurze Soundeffekte über miniaudio
class AudioPlayer
{
public:
    AudioPlayer();
    ~AudioPlayer();

    AudioPlayer(const AudioPlayer&) = delete;
    AudioPlayer& operator=(const AudioPlayer&) = delete;

    bool isReady() const;

    bool playSound(const std::string& filePath);

    void setMasterVolume(float volume);
    float masterVolume() const;

    bool startMusic(const std::string& filePath,
                    float endTrimSeconds = 1.9f,
                    float fadeDuration = 1.2f);

    void stopMusic();
    void update();

    bool isMusicPlaying() const;

private:
    ma_engine m_engine;
    ma_sound m_music;

    bool m_isReady;
    bool m_musicInitialized;

    float m_masterVolume;

    float m_musicLength;
    float m_musicLoopEnd;
    float m_musicFadeDuration;
};
