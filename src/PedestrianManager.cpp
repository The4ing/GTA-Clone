#include "PedestrianManager.h"
#include <cstdlib>

void PedestrianManager::spawnPedestrian(const sf::Vector2f& pos) {
    pedestrians.push_back(std::make_unique<Pedestrian>(pos));
}

void PedestrianManager::spawnMultiple(int count, const QuadTree<std::vector<sf::Vector2f>>& blockedPolyTree, int mapWidth, int mapHeight) {
    for (int i = 0; i < count; ++i) {
        sf::Vector2f pos = generateValidPosition(blockedPolyTree, mapWidth, mapHeight);
        spawnPedestrian(pos);
    }
}

void PedestrianManager::update(float dt, const QuadTree<std::vector<sf::Vector2f>>& blockedPolyTree) {
    for (auto& ped : pedestrians) {
        ped->update(dt, blockedPolyTree);
    }
}
void PedestrianManager::draw(sf::RenderTarget& target) const {
    for (const auto& ped : pedestrians) {
        ped->draw(target);
    }
}

const std::vector<std::unique_ptr<Pedestrian>>& PedestrianManager::getPedestrians() const {
    return pedestrians;
}

// Helpers
sf::Vector2f PedestrianManager::generateValidPosition(const QuadTree<std::vector<sf::Vector2f>>& blockedPolyTree, int mapWidth, int mapHeight, int maxAttempts) {
    for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        float x = static_cast<float>(rand() % mapWidth);
        float y = static_cast<float>(rand() % mapHeight);
        sf::Vector2f pos(x, y);
        if (!isBlocked(pos, blockedPolyTree)) {
            return pos;
        }
    }
    return { 100.f, 100.f }; // fallback if none found
}

bool PedestrianManager::isBlocked(const sf::Vector2f& pos, const QuadTree<std::vector<sf::Vector2f>>& blockedPolyTree) const {
    float radius = 10.f;
    sf::FloatRect area(pos.x - radius, pos.y - radius, radius * 2, radius * 2);
    auto nearby = blockedPolyTree.query(area);
    for (const auto& poly : nearby) {
        if (circleIntersectsPolygon(pos, radius, poly))
            return true;
    }
    return false;
}