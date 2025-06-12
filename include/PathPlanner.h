#pragma once
#include <vector>
#include <SFML/System.hpp>
#include "QuadTree.h"
#include "CollisionUtils.h"



std::vector<sf::Vector2f> findPathWorld(
    sf::Vector2f startPos,
    sf::Vector2f goalPos,
    float step,
    const QuadTree<std::vector<sf::Vector2f>>& blockedPolyTree,
    float radius,
    int maxNodes = 5000 
);
