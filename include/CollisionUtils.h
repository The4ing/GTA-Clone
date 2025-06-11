#pragma once
#include <SFML/System.hpp>
#include <vector>

bool circleIntersectsPolygon(const sf::Vector2f& circleCenter, float radius, const std::vector<sf::Vector2f>& polygon);
