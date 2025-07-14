#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class ControllersHelp {
public:
    ControllersHelp(sf::RenderWindow& window);
    void run();

private:
    sf::RenderWindow& window;
    sf::Sprite background;
    sf::Font font;
    std::vector<sf::Text> lines;

    void setupText();
};
