#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class GameHelp {
public:
    GameHelp(sf::RenderWindow& window);
    void run();

private:
    sf::RenderWindow& window;
    sf::Font font;
    std::vector<sf::Text> lines;
    sf::Sprite background;

    void setupText();
};
