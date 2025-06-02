#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class Slideshow {
public:
    Slideshow(sf::RenderWindow& window, float durationPerImage = 2.0f);

    void run();  // מציג את הסליידשואו

private:
    sf::RenderWindow& window;
    float durationPerImage;

    
    std::vector<std::string> imageKeys = {
        "SF",
        "SS",
    };
};
