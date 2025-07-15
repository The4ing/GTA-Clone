#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>

class Menu {
public:
    Menu(sf::RenderWindow& win);
    void update(sf::Event& event);
    void draw();
    int getSelectedIndex() const;
    bool isOptionChosen() const;
    std::string getSelectedOption() const;
    void reset();

private:
    sf::RenderWindow& window;
    sf::Font font;
    sf::Sprite background;
    std::vector<sf::Text> options;
    int selectedIndex = 0;
    bool optionChosen = false;

    sf::Sound menuLoop; // ✅ ניגון מוזיקת רקע
};
