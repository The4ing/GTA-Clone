#include "SoundManager.h"
#include "ResourceManager.h" // Ensure ResourceManager is included

SoundManager& SoundManager::getInstance() {
    static SoundManager instance;
    return instance;
}

SoundManager::SoundManager() : volume(100.f) {}

void SoundManager::playSound(const std::string& name) {
    removeStoppedSounds(); // Clean up sounds that have finished playing

    // Create a new sound and add it to the list
    sounds.emplace_back(ResourceManager::getInstance().getSoundBuffer(name));
    sounds.back().setVolume(volume);
    sounds.back().play();
}

void SoundManager::setVolume(float vol) {
    volume = std::min(100.f, std::max(0.f, vol)); // Clamp volume between 0 and 100
    for (auto& sound : sounds) {
        sound.setVolume(volume);
    }
}

float SoundManager::getVolume() const {
    return volume;
}

void SoundManager::removeStoppedSounds() {
    sounds.remove_if([](const sf::Sound& s) {
        return s.getStatus() == sf::Sound::Stopped;
        });
}