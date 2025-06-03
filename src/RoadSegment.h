#pragma once
#include <SFML/Graphics.hpp>
#include <string>

struct RoadSegment {
    sf::FloatRect bounds;
    std::string direction;  // "up", "down", "left", "right"
    int lanes = 1;
    bool is2D = false;

    sf::Vector2f getLaneCenter(int laneIndex) const {
        if (direction == "up" || direction == "down") {
            float laneWidth = bounds.width / lanes;
            float x = bounds.left + laneWidth * (laneIndex + 0.5f);
            float y = direction == "up" ? bounds.top + bounds.height : bounds.top;
            return { x, y };
        }
        else {
            float laneHeight = bounds.height / lanes;
            float x = direction == "left" ? bounds.left + bounds.width : bounds.left;
            float y = bounds.top + laneHeight * (laneIndex + 0.5f);
            return { x, y };
        }
    }
};
