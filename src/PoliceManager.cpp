#include "PoliceManager.h"
#include <limits>
#include <cmath>



void PoliceManager::spawnPolice(const sf::Vector2f& position) {
    policeUnits.push_back(std::make_unique<Police>(position));
}

void PoliceManager::update(float dt, const sf::Vector2f& playerPos, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    for (auto& unit : policeUnits) {
        unit->setTargetPosition(playerPos);
        unit->update(dt, blockedPolygons);

    }

    // הסרת שוטרים מתים
    policeUnits.erase(std::remove_if(policeUnits.begin(), policeUnits.end(),
        [](const std::unique_ptr<Police>& p) { return p->isDead(); }),
        policeUnits.end());
}

void PoliceManager::draw(sf::RenderWindow& window) {
    for (auto& unit : policeUnits) {
        unit->draw(window);
    }
}

void PoliceManager::damageClosest(const sf::Vector2f& pos, int amount) {
    float minDist = std::numeric_limits<float>::max();
    Police* closest = nullptr;

    for (auto& unit : policeUnits) {
        float dist = std::hypot(unit->getPosition().x - pos.x, unit->getPosition().y - pos.y);
        if (dist < minDist) {
            minDist = dist;
            closest = unit.get();
        }
    }

    if (closest)
        closest->takeDamage(amount);
}
