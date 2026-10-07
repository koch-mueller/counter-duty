
#include "AudioPlayer.h"

#include <algorithm>
#include <iostream>

namespace
{
    // Eine quadratische Kennlinie macht besonders kleine Lautstärkewerte feinfühliger
    float volumeToGain(float volume)
    {
        volume = std::clamp(volume, 0.0f, 1.0f);

        return volume * volume;
    }
}

AudioPlayer::AudioPlayer()
    : m_engine(),
      m_music(),
      m_isReady(false),
      m_musicInitialized(false),
      m_masterVolume(0.5f),
      m_musicLength(0.0f),
      m_musicLoopEnd(0.0f),
      m_musicFadeDuration(1.2f)
{
    ma_engine_config config = ma_engine_config_init();

    ma_result result = ma_engine_init(&config, &m_engine);

    if (result != MA_SUCCESS)
    {
        std::cout << "Audio-System konnte nicht initialisiert werden" << std::endl;

        return;
    }

    m_isReady = true;

    ma_engine_set_volume(&m_engine, volumeToGain(m_masterVolume));
}

AudioPlayer::~AudioPlayer()
{
    stopMusic();

    if (m_isReady)
    {
        ma_engine_uninit(&m_engine);
    }
}

bool AudioPlayer::isReady() const
{
    return m_isReady;
}

bool AudioPlayer::playSound(const std::string& filePath)
{
    if (!m_isReady)
    {
        return false;
    }

    ma_result result = ma_engine_play_sound(&m_engine, filePath.c_str(), nullptr);

    return result == MA_SUCCESS;
}

void AudioPlayer::setMasterVolume(float volume)
{
    m_masterVolume = std::clamp(volume, 0.0f, 1.0f);

    if (!m_isReady)
    {
        return;
    }

    ma_engine_set_volume(&m_engine, volumeToGain(m_masterVolume));
}

float AudioPlayer::masterVolume() const
{
    return m_masterVolume;
}

bool AudioPlayer::startMusic(const std::string& filePath, float endTrimSeconds, float fadeDuration)
{
    if (!m_isReady)
    {
        return false;
    }

    stopMusic();

    ma_result result = ma_sound_init_from_file(&m_engine,
                                               filePath.c_str(),
                                               MA_SOUND_FLAG_STREAM,
                                               nullptr,
                                               nullptr,
                                               &m_music);

    if (result != MA_SUCCESS)
    {
        std::cout << "Musik konnte nicht geladen werden: " << filePath << std::endl;

        return false;
    }

    m_musicInitialized = true;

    m_musicFadeDuration = std::max(fadeDuration, 0.0f);

    m_musicLength = 0.0f;

    ma_sound_get_length_in_seconds(&m_music, &m_musicLength);

    // Ein kurzer Bereich am Dateiende kann ausgelassen werden, falls dort Stille oder ein hörbarer Abschluss liegt, der beim Loopen stören würde
    m_musicLoopEnd = std::max(0.0f, m_musicLength - std::max(endTrimSeconds, 0.0f));

    if (m_musicLoopEnd <= 0.0f)
    {
        m_musicLoopEnd = m_musicLength;
    }

    ma_sound_set_looping(&m_music, MA_FALSE);

    ma_sound_set_volume(&m_music, 0.0f);

    result = ma_sound_start(&m_music);

    if (result != MA_SUCCESS)
    {
        stopMusic();

        return false;
    }

    return true;
}

void AudioPlayer::stopMusic()
{
    if (!m_musicInitialized)
    {
        return;
    }

    ma_sound_stop(&m_music);

    ma_sound_uninit(&m_music);

    m_musicInitialized = false;

    m_musicLength = 0.0f;
    m_musicLoopEnd = 0.0f;
}

void AudioPlayer::update()
{
    if (!m_musicInitialized)
    {
        return;
    }

    float cursor = 0.0f;

    if (ma_sound_get_cursor_in_seconds(&m_music, &cursor) != MA_SUCCESS)
    {
        return;
    }

    // Das Looping wird manuell gesteuert, damit Ein- und Ausblendung pro Schleife möglich sind
    if (m_musicLoopEnd > 0.0f && cursor >= m_musicLoopEnd)
    {
        ma_sound_seek_to_pcm_frame(&m_music, 0);

        ma_sound_set_volume(&m_music, 0.0f);

        return;
    }

    // Fade-In und Fade-Out treffen sich bei kurzen Loops über den jeweils kleineren Wert
    float musicVolume = 1.0f;

    if (m_musicFadeDuration > 0.0f && cursor < m_musicFadeDuration)
    {
        musicVolume = cursor / m_musicFadeDuration;
    }

    if (m_musicFadeDuration > 0.0f && m_musicLoopEnd > m_musicFadeDuration
        && cursor > m_musicLoopEnd - m_musicFadeDuration)
    {
        float fadeOutVolume = (m_musicLoopEnd - cursor) / m_musicFadeDuration;

        musicVolume = std::min(musicVolume, fadeOutVolume);
    }

    musicVolume = std::clamp(musicVolume, 0.0f, 1.0f);

    ma_sound_set_volume(&m_music, musicVolume);
}

bool AudioPlayer::isMusicPlaying() const
{
    if (!m_musicInitialized)
    {
        return false;
    }

    return ma_sound_is_playing(&m_music) == MA_TRUE;
}
