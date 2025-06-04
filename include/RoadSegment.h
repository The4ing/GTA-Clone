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
            float y = (direction == "up") ? bounds.top : (bounds.top + bounds.height);
            return { x, y };
        }
        else { // "left" or "right"
            float laneHeight = bounds.height / lanes;
            float x = (direction == "right") ? bounds.left : (bounds.left + bounds.width);
            float y = bounds.top + laneHeight * (laneIndex + 0.5f);
            return { x, y };
        }
    }

    sf::Vector2f getLaneEdge(int laneIndex, bool atStart) const {
        if (direction == "up") {
            float laneWidth = bounds.width / lanes;
            float x = bounds.left + laneWidth * (laneIndex + 0.5f);
            float y = atStart ? (bounds.top + bounds.height) : bounds.top;
            return { x, y };
        }
        if (direction == "down") {
            float laneWidth = bounds.width / lanes;
            float x = bounds.left + laneWidth * (laneIndex + 0.5f);
            float y = atStart ? bounds.top : (bounds.top + bounds.height);
            return { x, y };
        }
        if (direction == "left") {
            float laneHeight = bounds.height / lanes;
            float y = bounds.top + laneHeight * (laneIndex + 0.5f);
            float x = atStart ? (bounds.left + bounds.width) : bounds.left;
            return { x, y };
        }
        if (direction == "right") {
            float laneHeight = bounds.height / lanes;
            float y = bounds.top + laneHeight * (laneIndex + 0.5f);
            float x = atStart ? bounds.left : (bounds.left + bounds.width);
            return { x, y };
        }
        // fallback
        return { bounds.left, bounds.top };
    }



};
