#include "ResourceManager.h"
#include <stdexcept>

ResourceManager& ResourceManager::getInstance() {
    static ResourceManager instance;
    return instance;
}

// Texture
void ResourceManager::loadTexture(const std::string& name, const std::string& filepath) {
    sf::Texture texture;
    if (!texture.loadFromFile(filepath)) {
        throw std::runtime_error("Failed to load texture: " + filepath);
    }
    textures[name] = std::move(texture);
}

sf::Texture& ResourceManager::getTexture(const std::string& name) {
    auto it = textures.find(name);
    if (it == textures.end()) {
        throw std::runtime_error("Texture not found: " + name);
    }
    return it->second;
}

// Font
void ResourceManager::loadFont(const std::string& name, const std::string& filepath) {
    sf::Font font;
    if (!font.loadFromFile(filepath)) {
        throw std::runtime_error("Failed to load font: " + filepath);
    }
    fonts[name] = std::move(font);
}

sf::Font& ResourceManager::getFont(const std::string& name) {
    auto it = fonts.find(name);
    if (it == fonts.end()) {
        throw std::runtime_error("Font not found: " + name);
    }
    return it->second;
}

// SoundBuffer
void ResourceManager::loadSoundBuffer(const std::string& name, const std::string& filepath) {
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(filepath)) {
        throw std::runtime_error("Failed to load sound buffer: " + filepath);
    }
    soundBuffers[name] = std::move(buffer);
}

sf::SoundBuffer& ResourceManager::getSoundBuffer(const std::string& name) {
    auto it = soundBuffers.find(name);
    if (it == soundBuffers.end()) {
        throw std::runtime_error("Sound buffer not found: " + name);
    }
    return it->second;
}
