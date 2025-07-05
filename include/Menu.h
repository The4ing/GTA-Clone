#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class Menu {
public:
    Menu(sf::RenderWindow& window);

    void update(sf::Event& event);
    void draw();
    int getSelectedIndex() const;
    bool isOptionChosen() const;
    std::string getSelectedOption() const;
    void reset(); // Added reset method

private:


    sf::RenderWindow& window;
    sf::Font font;
    std::vector<sf::Text> options;
    int selectedIndex = 0;
    bool optionChosen = false;
    sf::Sprite background;
};
