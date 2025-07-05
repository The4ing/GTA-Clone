#include "PedestrianManager.h"
#include "CollisionUtils.h"
#include <cstdlib>

void PedestrianManager::spawnPedestrian(const sf::Vector2f& pos) {
    pedestrians.push_back(std::make_unique<Pedestrian>(pos));
}

void PedestrianManager::spawnMultiple(int count, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons, int mapWidth, int mapHeight) {
    for (int i = 0; i < count; ++i) {
        sf::Vector2f pos = generateValidPosition(blockedPolygons, mapWidth, mapHeight);
        spawnPedestrian(pos);
    }
}

void PedestrianManager::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    for (auto& ped : pedestrians) {
        ped->update(dt, blockedPolygons);
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
sf::Vector2f PedestrianManager::generateValidPosition(const std::vector<std::vector<sf::Vector2f>>& blockedPolygons, int mapWidth, int mapHeight, int maxAttempts) {
    for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        float x = static_cast<float>(rand() % mapWidth);
        float y = static_cast<float>(rand() % mapHeight);
        sf::Vector2f pos(x, y);
        if (!isBlocked(pos, blockedPolygons)) {
            return pos;
        }
    }
    return { 100.f, 100.f }; // fallback if none found
}

bool PedestrianManager::isBlocked(const sf::Vector2f& pos, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) const {
    return CollisionUtils::isInsideBlockedPolygon(pos, blockedPolygons);
}

