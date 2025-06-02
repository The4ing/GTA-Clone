#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class StoryScreen {
public:
    StoryScreen(sf::RenderWindow& window);

    void run();  

private:
    void handleEvents();
    void update();
    void render();

    sf::RenderWindow& window;
    sf::Font font;
    

    sf::Text storyText;
    std::string fullStory;
    std::string displayedText;
    size_t currentIndex;
    sf::Clock typingClock;
    bool done;
    sf::RectangleShape overlay;
    sf::Sprite background;
};
