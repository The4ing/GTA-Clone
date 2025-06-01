#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class Menu {
public:
    Menu(sf::RenderWindow& window);

    void update();
    void draw();
    int getSelectedIndex() const;
    bool isOptionChosen() const;
    std::string getSelectedOption() const;

private:
    void moveUp();
    void moveDown();

    sf::RenderWindow& window;
    sf::Font font;
    std::vector<sf::Text> options;
    int selectedIndex = 0;
    bool optionChosen = false;
};
