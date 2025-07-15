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
    sf::Sound& menuMusic; // הפניה למוזיקת התפריט לשליטה בעוצמה והשמעה

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

    // פונקציות עזר
    void setupOptions();   // יצירת הטקסטים של האפשרויות
    void setupBars();      // יצירת פסי ווליום ובהירות
    void updateBars();     // עדכון האורך והצבע של פסי השליטה

    // פונקציות של פעולות
    void toggleSound();
    void increaseVolume();
    void decreaseVolume();
    void increaseBrightness();
    void decreaseBrightness();
    void showHelp();
    void showStory();
};
