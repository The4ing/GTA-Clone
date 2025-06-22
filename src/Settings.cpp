#include "Settings.h"
#include "ResourceManager.h"
#include <iostream>

Settings::Settings(sf::RenderWindow& win)
    : window(win), soundEnabled(true), volumeLevel(1.0f), brightnessLevel(1.0f), selectedOption(-1)
{
    font = ResourceManager::getInstance().getFont("main");

    background.setTexture(ResourceManager::getInstance().getTexture("background_set"));

    sf::Vector2u textureSize = background.getTexture()->getSize();
    sf::Vector2u windowSize = window.getSize();

    background.setScale(
        static_cast<float>(windowSize.x) / textureSize.x,
        static_cast<float>(windowSize.y) / textureSize.y
    );

    background.setPosition(0.f, 0.f);
    

    setupOptions();
}
void Settings::setupOptions() {
    std::vector<std::string> texts = {
        "Story",         // ראשון, גודל גדול יותר
        "Help",
        "Toggle Music",
        "Volume +",
        "Volume -",
        "Brightness +",
        "Brightness -",
        "Back"
    };

    for (size_t i = 0; i < texts.size(); ++i) {
        sf::Text text;
        text.setFont(font);
        text.setString(texts[i]);
        text.setCharacterSize(i == 0 ? 42 : 32);  // קצת יותר גדול
        text.setFillColor(sf::Color::White);
        text.setPosition(100, 100 + i * 60);
        options.push_back(text);
    }
}

void Settings::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        auto mousePos = sf::Mouse::getPosition(window);
        for (size_t i = 0; i < options.size(); ++i) {
            if (options[i].getGlobalBounds().contains((float)mousePos.x, (float)mousePos.y)) {
                selectedOption = (int)i;
                switch (selectedOption) {
                case 0: showStory(); break;        // Story
                case 1: showHelp(); break;         // Help
                case 2: toggleSound(); break;      // Toggle Music
                case 3: increaseVolume(); break;
                case 4: decreaseVolume(); break;
                case 5: increaseBrightness(); break;
                case 6: decreaseBrightness(); break;
                case 7: shouldClose = true; break;           // Back
                }
            }
        }
    }
}


void Settings::update() {
    auto mousePos = sf::Mouse::getPosition(window);
    for (auto& text : options) {
        if (text.getGlobalBounds().contains((float)mousePos.x, (float)mousePos.y)) {
            text.setFillColor(sf::Color::Yellow);
            text.setCharacterSize(40); 
        }
        else {
            text.setFillColor(sf::Color::White);
            text.setCharacterSize(32);  
        }
    }
}


void Settings::draw() {
    window.draw(background);
    for (auto& text : options)
        window.draw(text);
} 

// פעולות
void Settings::toggleSound() {
    soundEnabled = !soundEnabled;
}

void Settings::increaseVolume() {
    volumeLevel += 0.1f;
    if (volumeLevel > 1.0f) volumeLevel = 1.0f;
}

void Settings::decreaseVolume() {
    volumeLevel -= 0.1f;
    if (volumeLevel < 0.0f) volumeLevel = 0.0f;
}

void Settings::increaseBrightness() {
    brightnessLevel += 0.1f;
    if (brightnessLevel > 1.0f) brightnessLevel = 1.0f;
   // window.setBrightness(static_cast<int>(brightnessLevel * 255)); 
}

void Settings::decreaseBrightness() {
    brightnessLevel -= 0.1f;
    if (brightnessLevel < 0.0f) brightnessLevel = 0.0f;
    //window.setBrightness(static_cast<int>(brightnessLevel * 255)); 
}

void Settings::showHelp() {
    //std::cout << "This is a game where you must survive and complete levels.\n";
}

bool Settings::shouldExit() const {
    return shouldClose;
}

void Settings::showStory() {
    StoryScreen story(window);
    story.run();
}
