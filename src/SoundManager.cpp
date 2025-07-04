#include "SoundManager.h"
#include "ResourceManager.h"
#include <cstdlib> // For std::rand
#include <algorithm> // For std::min/max
#include <iostream> // For debugging

SoundManager& SoundManager::getInstance() {
    static SoundManager instance;
    return instance;
}

// Initialize volume to a non-zero default, e.g., 70%.
SoundManager::SoundManager() : volume(70.f), volumeBeforeMute(70.f), muted(false) {}

void SoundManager::playSound(const std::string& name, float pitch) {
    removeStoppedSounds();

    sounds.emplace_back(ResourceManager::getInstance().getSoundBuffer(name));
    sounds.back().setVolume(muted ? 0.f : volume); // Apply effective volume
    sounds.back().setPitch(pitch);
    sounds.back().play();
    // std::cout << "Playing sound: " << name << " at volume: " << (muted ? 0.f : volume) << std::endl;
}

void SoundManager::playRandomSound(const std::vector<std::string>& names,
    float minPitch, float maxPitch) {
    if (names.empty()) return;

    int index = std::rand() % names.size();
    float pitch = minPitch;
    if (maxPitch > minPitch && RAND_MAX > 0) { // check RAND_MAX to avoid division by zero if it's misconfigured
        float t = static_cast<float>(std::rand()) / RAND_MAX;
        pitch = minPitch + t * (maxPitch - minPitch);
    }
    playSound(names[index], pitch);
}

// This is the primary method to change the target volume.
// It also handles applying the volume to currently playing sounds if not muted.
void SoundManager::setVolume(float vol) {
    volume = std::min(100.f, std::max(0.f, vol)); // Clamp to 0-100

    if (!muted) { // Only apply to sounds if not muted
        for (auto& sound : sounds) {
            sound.setVolume(volume);
        }
    }
    // If muted, 'volume' is updated, but sounds remain at 0 until unmuted.
    // std::cout << "SoundManager::setVolume to " << volume << ". Muted: " << muted << std::endl;
}

void SoundManager::increaseVolume(float step) {
    // std::cout << "Attempting to increase volume. Current: " << volume << ", Muted: " << muted << ", Step: " << step << std::endl;
    float newVolume = std::min(100.f, volume + step);

    if (muted && newVolume > 0) {
        // If was muted and increasing volume would make it audible, then unmute.
        // The act of increasing volume implies the user wants to hear sound.
        muted = false;
        // volume is already set to volumeBeforeMute by toggleMute logic if we called it.
        // Here, we are directly setting muted = false, so 'volume' should be our new target.
    }
    setVolume(newVolume); // This will apply the newVolume if not muted.
    // If we just unmuted, it will apply newVolume.
}

void SoundManager::decreaseVolume(float step) {
    // std::cout << "Attempting to decrease volume. Current: " << volume << ", Muted: " << muted << ", Step: " << step << std::endl;
    float newVolume = std::max(0.f, volume - step);
    setVolume(newVolume); // Set the new target volume

    // If volume is decreased to 0, it's just 0. It doesn't automatically toggle the 'muted' state.
    // The 'muted' state is for preserving a previous volume level.
    // If user decreases to 0, then increases, it increases from 0.
    // If user mutes (at e.g. 50), then unmuted, it goes back to 50.
}

void SoundManager::toggleMute() {
    muted = !muted;
    // std::cout << "SoundManager::toggleMute. Now muted: " << muted << std::endl;
    if (muted) {
        volumeBeforeMute = volume; // Save current volume when muting
        for (auto& sound : sounds) {
            sound.setVolume(0.f); // Silence all sounds
        }
    }
    else {
        // Unmuting: restore volume to the level it was *before* this mute action.
        volume = volumeBeforeMute;
        for (auto& sound : sounds) {
            sound.setVolume(volume); // Apply restored volume
        }
    }
}

bool SoundManager::isMuted() const {
    return muted;
}

// Returns the current target volume level (0-100).
// This is what a volume slider in UI would typically represent.
float SoundManager::getVolume() const {
    return volume;
}
// If you need a function for "effective/audible volume":
// float SoundManager::getAudibleVolume() const {
//     return muted ? 0.f : volume;
// }


void SoundManager::removeStoppedSounds() {
    sounds.remove_if([](const sf::Sound& s) {
        return s.getStatus() == sf::Sound::Stopped;
        });
}