#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>

class Settings {
public:
   
    Settings(sf::RenderWindow& win, sf::Sound& menuSound);

    void handleEvent(const sf::Event& event); 
    void update();                          
    void draw();                             
    bool shouldExit() const;                

private:
    sf::RenderWindow& window;
    sf::Sound& menuMusic; 

    sf::Font font;
    sf::Sprite background;
    sf::RectangleShape volumeBar;
    sf::RectangleShape brightnessBar;
    sf::RectangleShape brightnessOverlay;
    sf::Text musicStatusText;


    std::vector<sf::Text> options;
    bool soundEnabled;
    float volumeLevel;
    float brightnessLevel;
    int selectedOption;
    bool shouldClose = false;

    void setupOptions();  
    void setupBars();      
    void updateBars();    

    void toggleSound();
    void increaseVolume();
    void decreaseVolume();
    void increaseBrightness();
    void decreaseBrightness();
    void showHelp();
    void showStory();
};
