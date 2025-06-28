#include "PoliceManager.h"
#include <limits>
#include <cmath>
#include <iostream>
#include "GameManager.h" // Required for GameManager type
// Initialize static members for pathfinding throttling
int s_pathfindingCallsThisFrame = 0;
const int S_MAX_PATHFINDING_CALLS_PER_FRAME = 4; // Example: Max 4 calls per frame

PoliceManager::PoliceManager(GameManager& gameManager) : m_gameManager(gameManager) {} // Initialize GameManager reference

bool PoliceManager::canRequestPath() {
    return s_pathfindingCallsThisFrame < S_MAX_PATHFINDING_CALLS_PER_FRAME;
}

void PoliceManager::recordPathfindingCall() {
    s_pathfindingCallsThisFrame++;
}

void PoliceManager::resetPathfindingCounter() {
    s_pathfindingCallsThisFrame = 0;
}

void PoliceManager::spawnPolice(const sf::Vector2f& position) {
    // Pass the GameManager reference to the Police constructor
    policeUnits.push_back(std::make_unique<Police>(m_gameManager));
    // Set position after creation, or modify Police constructor if it needs position directly and GM.
    // For now, Police constructor only takes GM, position is set if needed by other means (e.g. a setInitialPosition method or default)
    // The current Police constructor does not take position, it's set internally or defaults.
    // Let's ensure new police units are positioned correctly.
    // The Police constructor sets a default position. We should override it.
    if (!policeUnits.empty()) {
        policeUnits.back()->setPosition(position); // Set the position for the newly spawned police
    }
    //std::cout << "Spawned police at: (" << position.x << ", " << position.y << ")\n";
}


void PoliceManager::update(float dt, const sf::Vector2f& playerPos,
    const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    spawnCooldown -= dt;
    resetPathfindingCounter(); // Reset counter at the beginning of each frame's update

    // ... (rest of the spawning logic if any) ...

    for (auto& unit : policeUnits) {
        // unit->setTargetPosition(playerPos); // This is now done inside Police::update
        unit->update(dt, playerPos, blockedPolygons); // Pass playerPos to Police::update
    }

    policeUnits.erase(std::remove_if(policeUnits.begin(), policeUnits.end(),
        [](const std::unique_ptr<Police>& p) { return p->isDead(); }),
        policeUnits.end());
}




void PoliceManager::draw(sf::RenderTarget& window) {
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