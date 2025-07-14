#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Help {
public:
    Help(sf::RenderWindow& window);
    void run();

private:
    sf::RenderWindow& window;
    sf::Font font;
    sf::Sprite background;
    std::vector<sf::Text> menuOptions;
    int selectedOption;
    bool shouldClose ;

    void setupMenu();
    void handleInput();
    void updateVisuals();
};
