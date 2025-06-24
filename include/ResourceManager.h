#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>

class ResourceManager {
public:
    static ResourceManager& getInstance();

    // Texture functions
    void loadTexture(const std::string& name, const std::string& filepath);
    sf::Texture& getTexture(const std::string& name);

    // Font functions ✅
    void loadFont(const std::string& name, const std::string& filepath);
    sf::Font& getFont(const std::string& name);

    // SoundBuffer functions
    void loadSoundBuffer(const std::string& name, const std::string& filepath);
    sf::SoundBuffer& getSoundBuffer(const std::string& name);

private:
    ResourceManager() = default;

    std::unordered_map<std::string, sf::Texture> textures;
    std::unordered_map<std::string, sf::Font> fonts;
    std::unordered_map<std::string, sf::SoundBuffer> soundBuffers;
};
