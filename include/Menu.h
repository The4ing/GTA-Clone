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
    void reset();

    int getSelectedIndex() const;
    bool isOptionChosen() const;
    
    sf::Sound& getMenuLoop();
    std::string getSelectedOption() const;

private:
    sf::RenderWindow& window;
    sf::Font font;
    sf::Sprite background;
    std::vector<sf::Text> options;
    int selectedIndex ;
    bool optionChosen;
    sf::Sound menuLoop; 
};
