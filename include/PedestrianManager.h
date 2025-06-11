#pragma once
#include "Pedestrian.h"
#include <vector>
#include <memory>

class PedestrianManager {
public:
    PedestrianManager() = default;

    void spawnPedestrian(const sf::Vector2f& pos);
    void spawnMultiple(int count, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons, int mapWidth, int mapHeight);

    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void draw(sf::RenderTarget& target) const;

    const std::vector<std::unique_ptr<Pedestrian>>& getPedestrians() const;

private:
    std::vector<std::unique_ptr<Pedestrian>> pedestrians;

    sf::Vector2f generateValidPosition(const std::vector<std::vector<sf::Vector2f>>& blockedPolygons, int mapWidth, int mapHeight, int maxAttempts = 100);
    bool isBlocked(const sf::Vector2f& pos, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) const;
};
