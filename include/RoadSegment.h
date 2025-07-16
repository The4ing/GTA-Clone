#pragma once
#include <SFML/Graphics.hpp>
#include <string>

struct RoadSegment {
    sf::FloatRect bounds;
    std::string direction;  // "up", "down", "left", "right"
    int lanes = 1;
    bool is2D = false;

    sf::Vector2f getLaneCenter(int laneIndex) const {
        int realLanes = lanes;
        std::string dir = direction;
        bool reverse = false;
        if (is2D && lanes > 1) {
            int half = lanes / 2;
            if (laneIndex < half) {
                
                reverse = true;
            }
        }

        if (dir == "up" || dir == "down") {
            float laneWidth = bounds.width / realLanes;
            float x = bounds.left + laneWidth * (laneIndex + 0.5f);
            float y;
            if ((dir == "up" && !reverse) || (dir == "down" && reverse))
                y = bounds.top + bounds.height;
            else
                y = bounds.top;
            return { x, y };
        }
        else {
            float laneHeight = bounds.height / realLanes;
            float y = bounds.top + laneHeight * (laneIndex + 0.5f);
            float x;
            if ((dir == "right" && !reverse) || (dir == "left" && reverse))
                x = bounds.left;
            else
                x = bounds.left + bounds.width;
            return { x, y };
        }
    }


    sf::Vector2f getLaneEdge(int laneIndex, bool atStart) const {
        int realLanes = lanes;
        std::string dir = direction;
        bool reverse = false;
        if (is2D && lanes > 1) {
            int half = lanes / 2;
            if (laneIndex < half) {
                reverse = true;
            }
        }

        bool effectiveAtStart = reverse ? !atStart : atStart;

        if (dir == "up") {
            float laneWidth = bounds.width / realLanes;
            float x = bounds.left + laneWidth * (laneIndex + 0.5f);
            float y = effectiveAtStart ? (bounds.top + bounds.height) : bounds.top;
            return { x, y };
        }
        if (dir == "down") {
            float laneWidth = bounds.width / realLanes;
            float x = bounds.left + laneWidth * (laneIndex + 0.5f);
            float y = effectiveAtStart ? bounds.top : (bounds.top + bounds.height);
            return { x, y };
        }
        if (dir == "left") {
            float laneHeight = bounds.height / realLanes;
            float y = bounds.top + laneHeight * (laneIndex + 0.5f);
            float x = effectiveAtStart ? (bounds.left + bounds.width) : bounds.left;
            return { x, y };
        }
        if (dir == "right") {
            float laneHeight = bounds.height / realLanes;
            float y = bounds.top + laneHeight * (laneIndex + 0.5f);
            float x = effectiveAtStart ? bounds.left : (bounds.left + bounds.width);
            return { x, y };
        }
        // fallback
        return { bounds.left, bounds.top };
    }

};
