#include "PoliceManager.h"
#include <limits>
#include <cmath>



void PoliceManager::spawnPolice(const sf::Vector2f& position) {
    policeUnits.push_back(std::make_unique<Police>(position));
}

void PoliceManager::update(float dt, const sf::Vector2f& playerPos,
    const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    spawnCooldown -= dt;

    // יצירה אקראית ליד צ’אנקים פעילים (אחת ל־5 שניות נניח)
    if (spawnCooldown <= 0.f) {
        for (const auto& chunk : activeChunks) {
            if (rand() % 100 < 25) { // 25% סיכוי ליצור שוטר
                spawnPoliceNearChunk(chunk);
            }
        }
        spawnCooldown = 5.f;
    }

    for (auto& unit : policeUnits) {
        unit->setTargetPosition(playerPos);
        unit->update(dt, blockedPolygons);
    }

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

void PoliceManager::spawnPoliceNearChunk(const sf::Vector2i& chunkCoord) {
    float chunkSize = 256.f; // לדוגמה
    sf::Vector2f basePos(chunkCoord.x * chunkSize, chunkCoord.y * chunkSize);
    sf::Vector2f offset(rand() % 100 - 50, rand() % 100 - 50);
    spawnPolice(basePos + offset);
}



void PoliceManager::trySpawnRandomPoliceNear(const std::vector<sf::Vector2i>& activeChunks, const sf::Vector2f& playerPos) {
    static float cooldown = 0.f;
    cooldown -= 1.f / 60.f;

    if (cooldown > 0.f) return;
    cooldown = 7.f; // כ־7 שניות בין בדיקות

    if (activeChunks.empty()) return;

    if (rand() % 100 < 60) return; // 60% סיכוי לדלג

    const int chunkSize = CHUNK_SIZE;
    sf::Vector2i chunk = activeChunks[rand() % activeChunks.size()];
    sf::Vector2f pos = {
        chunk.x * chunkSize + static_cast<float>(rand() % chunkSize),
        chunk.y * chunkSize + static_cast<float>(rand() % chunkSize)
    };

    float dist = std::hypot(playerPos.x - pos.x, playerPos.y - pos.y);
    if (dist < 150.f || dist > 900.f) return;

    spawnPolice(pos);
}



