#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>

class Settings {
public:
    // קונסטרקטור שמקבל גם את חלון המשחק וגם את המוזיקה של התפריט כדי לשלוט עליה
    Settings(sf::RenderWindow& win, sf::Sound& menuSound);

    void handleEvent(const sf::Event& event); // טיפול בלחיצות
    void update();                            // שינוי צבע וגודל הטקסטים כשהעכבר מעל
    void draw();                              // ציור המסך
    bool shouldExit() const;                 // האם לצאת ממסך ההגדרות

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
