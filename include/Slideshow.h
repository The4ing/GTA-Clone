#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "SoundManager.h"

class Slideshow {
public:
    Slideshow(sf::RenderWindow& window, float durationPerImage = 2.0f);

    void run();  

private:
     
    sf::RenderWindow& window;
    float durationPerImage;

    
    std::vector<std::string> imageKeys = {
        "SF",
        "SS",
        "ST",
    };
};
