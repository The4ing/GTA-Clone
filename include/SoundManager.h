#pragma once
#include <SFML/Audio.hpp>
#include <string>
#include <list>
#include "ResourceManager.h"

class SoundManager {
public:
    static SoundManager& getInstance();

    void playSound(const std::string& name);
    void setVolume(float volume);
    float getVolume() const;
    void removeStoppedSounds();

private:
    SoundManager();
    ~SoundManager() = default;

    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    std::list<sf::Sound> sounds;
    float volume;
};