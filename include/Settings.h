#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "StoryScreen.h"
#include "Help.h"

class Settings {
public:
    Settings(sf::RenderWindow& win);

    void handleEvent(const sf::Event& event);
    void update();
    void draw();
    bool shouldExit() const;
    void showStory();

private:
    sf::RenderWindow& window;
    sf::Font font;
    sf::Sprite background;
    

    bool shouldClose = false;
    bool soundEnabled;
    float volumeLevel;
    float brightnessLevel; 

    std::vector<sf::Text> options;
    int selectedOption;

    void toggleSound();
    void increaseVolume();
    void decreaseVolume();
    void increaseBrightness();
    void decreaseBrightness();
    void showHelp();
    

    void setupOptions();
    //void updateOptionTexts();
};
