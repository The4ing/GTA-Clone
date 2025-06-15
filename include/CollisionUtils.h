#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class CollisionUtils {
public:
    static bool pointInPolygon(const sf::Vector2f& point, const std::vector<sf::Vector2f>& polygon);
};
