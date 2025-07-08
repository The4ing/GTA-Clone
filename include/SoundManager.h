#pragma once
#include <SFML/Audio.hpp>
#include <string>
#include <list>
#include "ResourceManager.h"
#include <vector>

class SoundManager {
public:
    static SoundManager& getInstance();

    void playSound(const std::string& name, float pitch = 1.f);
    void playRandomSound(const std::vector<std::string>& names,
        float minPitch = 1.f, float maxPitch = 1.f);
    void setVolume(float volume);
    void increaseVolume(float step = 5.f);
    void decreaseVolume(float step = 5.f);
    void toggleMute();
    bool isMuted() const;
    float getVolume() const;
    void removeStoppedSounds();
    void playWantedLoop(float volume);
    void stopWantedLoop();
    void pauseAll();
    void resumeAll();
    bool isWantedLoopPlaying() const;

private:
    SoundManager();
    ~SoundManager() = default;

    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    std::list<sf::Sound> sounds;
    float volume;
    float volumeBeforeMute;
    bool muted;
    sf::Sound m_wantedLoop;
};