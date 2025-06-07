#pragma once

#include "Police.h"
#include <vector>
#include <memory>

class PoliceManager {
public:
    PoliceManager() = default;
    void spawnPolice(const sf::Vector2f& position);
    void update(float dt, const sf::Vector2f& playerPos, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void draw(sf::RenderWindow& window);
    void damageClosest(const sf::Vector2f& pos, int amount);

private:
    std::vector<std::unique_ptr<Police>> policeUnits;
};
