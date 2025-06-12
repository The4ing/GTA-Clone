#pragma once
#include "Pedestrian.h"
#include <vector>
#include <memory>
#include "QuadTree.h"
#include "CollisionUtils.h"

class PedestrianManager {
public:
    PedestrianManager() = default;

    void spawnPedestrian(const sf::Vector2f& pos);
    void spawnMultiple(int count, const QuadTree<std::vector<sf::Vector2f>>& blockedPolyTree, int mapWidth, int mapHeight);
    void update(float dt, const QuadTree<std::vector<sf::Vector2f>>& blockedPolyTree);
    void draw(sf::RenderTarget& target) const;

    const std::vector<std::unique_ptr<Pedestrian>>& getPedestrians() const;

private:
    std::vector<std::unique_ptr<Pedestrian>> pedestrians;

    sf::Vector2f generateValidPosition(const QuadTree<std::vector<sf::Vector2f>>& blockedPolyTree, int mapWidth, int mapHeight, int maxAttempts = 100);
    bool isBlocked(const sf::Vector2f& pos, const QuadTree<std::vector<sf::Vector2f>>& blockedPolyTree) const;
};
