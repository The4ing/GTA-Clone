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

void SoundManager::registerExternalSound(sf::Sound* sound) {
    if (sound &&
        std::find(m_externalSounds.begin(), m_externalSounds.end(), sound) == m_externalSounds.end()) {
        m_externalSounds.push_back(sound);
    }
}

void SoundManager::unregisterExternalSound(sf::Sound* sound) {
    m_externalSounds.remove(sound);
}

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
    if (maxPitch > minPitch && RAND_MAX > 0) { 
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
    
}

void SoundManager::increaseVolume(float step) {
   
    float newVolume = std::min(100.f, volume + step);

    if (muted && newVolume > 0) {
        muted = false;
        
    }
    setVolume(newVolume); // This will apply the newVolume if not muted.
    // If we just unmuted, it will apply newVolume.
}

void SoundManager::decreaseVolume(float step) {
    
    float newVolume = std::max(0.f, volume - step);
    setVolume(newVolume); // Set the new target volume

    
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

float SoundManager::getVolume() const {
    return volume;
}



void SoundManager::removeStoppedSounds() {
    sounds.remove_if([](const sf::Sound& s) {
        return s.getStatus() == sf::Sound::Stopped;
        });
}

void SoundManager::pauseAll() {
    for (auto& s : sounds) {
        if (s.getStatus() == sf::Sound::Playing)
            s.pause();
    }
    for (auto* ext : m_externalSounds) {
        if (ext && ext->getStatus() == sf::Sound::Playing)
            ext->pause();
    }
    if (m_wantedLoop.getStatus() == sf::Sound::Playing)
        m_wantedLoop.pause();
    std::cout << "paused all sounds";
}

void SoundManager::resumeAll() {
    for (auto& s : sounds) {
        if (s.getStatus() == sf::Sound::Paused)
            s.play();
    }
    for (auto* ext : m_externalSounds) {
        if (ext && ext->getStatus() == sf::Sound::Paused)
            ext->play();
    }
    if (m_wantedLoop.getStatus() == sf::Sound::Paused)
        m_wantedLoop.play();
}

void SoundManager::playWantedLoop(float volumeLevel) {
    if (m_wantedLoop.getStatus() != sf::Sound::Playing) {
        m_wantedLoop.setBuffer(ResourceManager::getInstance().getSoundBuffer("wanted"));
        m_wantedLoop.setLoop(true);
       // SoundManager::getInstance().registerExternalSound(&m_sirenSound);
        m_wantedLoop.play();
    }
    float effectiveVol = muted ? 0.f : volumeLevel;
    m_wantedLoop.setVolume(effectiveVol);
}

void SoundManager::stopWantedLoop() {
    if (m_wantedLoop.getStatus() == sf::Sound::Playing) {
        m_wantedLoop.stop();
    }
}

bool SoundManager::isWantedLoopPlaying() const {
    return m_wantedLoop.getStatus() == sf::Sound::Playing;
}

void SoundManager::playLoopingSound(const std::string& name, sf::Sound& soundOut) {
    soundOut.setBuffer(ResourceManager::getInstance().getSoundBuffer(name));
    soundOut.setLoop(true);
    float effectiveVol = muted ? 0.f : volume;
    soundOut.setVolume(effectiveVol);
    soundOut.play();
    registerExternalSound(&soundOut);
}
