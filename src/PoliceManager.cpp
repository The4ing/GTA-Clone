#include "PoliceManager.h"
#include "GameManager.h"
#include "Player.h" // Include Player header
#include <limits>
#include <cmath>
#include <iostream> // For std::cout, std::cerr
#include <random>   // For std::mt19937, std::uniform_int_distribution

// Initialize static members for pathfinding throttling
int s_pathfindingCallsThisFrame = 0;
const int S_MAX_PATHFINDING_CALLS_PER_FRAME = 10; // Increased slightly, can be tuned

PoliceManager::PoliceManager(GameManager& gameManager) : m_gameManager(gameManager) {
    // Seed for random number generation
    // generator.seed(std::random_device{}()); // This was commented out, ensure it's initialized if used.
}

bool PoliceManager::canRequestPath() {
    return s_pathfindingCallsThisFrame < S_MAX_PATHFINDING_CALLS_PER_FRAME;
}

void PoliceManager::recordPathfindingCall() {
    s_pathfindingCallsThisFrame++;
}

void PoliceManager::resetPathfindingCounter() {
    s_pathfindingCallsThisFrame = 0;
}

// Spawn a police officer with a specific weapon type
void PoliceManager::spawnPolice(const sf::Vector2f& position, PoliceWeaponType weaponType) {
    m_policeOfficers.push_back(std::make_unique<Police>(m_gameManager, weaponType));
    if (!m_policeOfficers.empty()) {
        m_policeOfficers.back()->setPosition(position);
        // std::cout << "Spawned " << (weaponType == PoliceWeaponType::BATON ? "Baton Police" : "Pistol Police")
        //           << " at: (" << position.x << ", " << position.y << ")\n";
    }
}

// Spawn a police car
void PoliceManager::spawnPoliceCar(const sf::Vector2f& position) {
    m_policeCars.push_back(std::make_unique<PoliceCar>(m_gameManager, position));
    if (!m_policeCars.empty()) {
        m_policeCars.back()->setIsAmbient(false); // Ensure cars spawned this way are aggressive
        // std::cout << "Spawned AGGRESSIVE Police Car at: (" << position.x << ", " << position.y << ")\n";
    }
}

void PoliceManager::spawnAmbientPoliceCarOnRoadSegment(const RoadSegment* road, int laneIndex, const std::string& actualDir, const sf::Vector2f& spawnPosition) {
    auto newPoliceCar = std::make_unique<PoliceCar>(m_gameManager, spawnPosition);

    // It seems PoliceCar constructor handles its own texture ("PoliceCar") and scale (0.09f).
    // If specific textures like "policeCar1" are needed, or different scales, set them here.
    // newPoliceCar->setTexture(ResourceManager::getInstance().getTexture("policeCar1")); // Example if needed
    // newPoliceCar->setScale(0.05f, 0.05f); // Example if different scale needed

    newPoliceCar->setPosition(spawnPosition); // Redundant if constructor does it, but safe.
    newPoliceCar->setDirectionVec(actualDir);
    newPoliceCar->setCurrentRoad(road);
    newPoliceCar->setCurrentLaneIndex(laneIndex);

    newPoliceCar->setIsAmbient(true);
    newPoliceCar->m_playerCausedWantedIncrease = false; // Explicitly set, though it's the default

    m_policeCars.push_back(std::move(newPoliceCar));
    // std::cout << "Spawned AMBIENT Police Car at: (" << spawnPosition.x << ", " << spawnPosition.y << ")\n";
}

void PoliceManager::update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    resetPathfindingCounter();

    // Update spawn timers
    if (m_batonOfficerSpawnTimer > 0.f) m_batonOfficerSpawnTimer -= dt;
    if (m_pistolOfficerSpawnTimer > 0.f) m_pistolOfficerSpawnTimer -= dt;
    if (m_policeCarSpawnTimer > 0.f) m_policeCarSpawnTimer -= dt;
    if (m_policeHelicopterSpawnTimer > 0.f) m_policeHelicopterSpawnTimer -= dt; // Added
    if (m_policeTankSpawnTimer > 0.f) m_policeTankSpawnTimer -= dt;             // Added

    int wantedLevel = player.getWantedLevel();
    managePolicePopulation(wantedLevel, player.getPosition());

    updatePoliceOfficers(dt, player, blockedPolygons); // Renamed from updatePoliceUnits
    updatePoliceCars(dt, player, blockedPolygons);
    updatePoliceHelicopters(dt, player, blockedPolygons); // Added
    updatePoliceTanks(dt, player, blockedPolygons);       // Added

    // Old random spawning logic (trySpawnRandomPoliceNear) might be disabled or integrated into managePolicePopulation for 0 stars.
    // if (wantedLevel == 0) {
    //    trySpawnRandomPoliceNear(m_gameManager.getActiveChunks(), player.getPosition()); // Assuming GameManager provides activeChunks
    // }
}

// Renamed from updatePoliceUnits
void PoliceManager::updatePoliceOfficers(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    for (auto& unit : m_policeOfficers) {
        unit->update(dt, player, blockedPolygons);
    }
    m_policeOfficers.erase(std::remove_if(m_policeOfficers.begin(), m_policeOfficers.end(),
        [](const std::unique_ptr<Police>& p) { return p->isDead(); }),
        m_policeOfficers.end());
}

void PoliceManager::updatePoliceCars(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    for (auto& car : m_policeCars) {
        car->update(dt, player, blockedPolygons);
    }
    // Assuming PoliceCar has an isDestroyed method or similar (e.g. based on health)
    m_policeCars.erase(std::remove_if(m_policeCars.begin(), m_policeCars.end(),
        [](const std::unique_ptr<PoliceCar>& pc) { return pc->isDestroyed(); }),
        m_policeCars.end());
}

void PoliceManager::updatePoliceHelicopters(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    for (auto& heli : m_policeHelicopters) {
        heli->update(dt, player, blockedPolygons); // Pass blockedPolygons if helicopters need to avoid tall buildings
    }
    m_policeHelicopters.erase(std::remove_if(m_policeHelicopters.begin(), m_policeHelicopters.end(),
        [](const std::unique_ptr<PoliceHelicopter>& ph) { return ph->isDestroyed(); }),
        m_policeHelicopters.end());
}

void PoliceManager::updatePoliceTanks(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    for (auto& tank : m_policeTanks) {
        tank->update(dt, player, blockedPolygons);
    }
    m_policeTanks.erase(std::remove_if(m_policeTanks.begin(), m_policeTanks.end(),
        [](const std::unique_ptr<PoliceTank>& pt) { return pt->isDestroyed(); }),
        m_policeTanks.end());
}


void PoliceManager::managePolicePopulation(int wantedLevel, const sf::Vector2f& playerPos) {
    m_desiredBatonOfficers = 0;
    m_desiredPistolOfficers = 0;
    m_desiredPoliceCars = 0;
    m_desiredPoliceHelicopters = 0; // Initialize
    m_desiredPoliceTanks = 0;       // Initialize

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(1, 10);

    // Determine desired counts based on wanted level
    switch (wantedLevel) {
    case 0:
        // No aggressive police. Existing ones might retreat or despawn if too far from original posts.
        // Or they just wander idly. For now, we just don't spawn new aggressive ones.
        // If any aggressive units exist from higher wanted levels, they should be handled (e.g. set to Idle, despawned)
        // This part needs more thought: how to make existing units non-aggressive.
        // For now, we just stop spawning new ones and let existing ones behave as they do.
        // A simple approach: if wanted level drops to 0, clear all *aggressive* police.
        // clearAllPolice(); // This is too aggressive, they should just calm down.
        // For now, they will continue their last order (e.g. Chasing if they were) until they lose the player.
        break;
    case 1:
        m_desiredBatonOfficers = dist(rng);
        break;
    case 2:
        m_desiredBatonOfficers = dist(rng);
        m_desiredPistolOfficers = dist(rng);
        break;
    case 3:
        m_desiredBatonOfficers = dist(rng);
        m_desiredPistolOfficers = dist(rng);
        m_desiredPoliceCars = dist(rng);
        break;
    case 4: // Includes 3-star + helicopters (not implemented yet)
        m_desiredBatonOfficers = dist(rng);
        m_desiredPistolOfficers = dist(rng);
        m_desiredPoliceCars = dist(rng);
        m_desiredPoliceHelicopters = dist(rng); // Added
        break;
    case 5:
        m_desiredBatonOfficers = dist(rng);
        m_desiredPistolOfficers = dist(rng);
        m_desiredPoliceCars = dist(rng);
        m_desiredPoliceHelicopters = dist(rng); // Added
        m_desiredPoliceTanks = dist(rng);       // Added
        break;
    default:
        if (wantedLevel > 5) { // Treat >5 as 5-star max
            m_desiredBatonOfficers = 10;
            m_desiredPistolOfficers = 10;
            m_desiredPoliceCars = 10;
            m_desiredPoliceHelicopters = 10; // Max out
            m_desiredPoliceTanks = 10;       // Max out
        }
        break;
    }

    // Spawn Baton Officers
    if (countPoliceByType(PoliceWeaponType::BATON) < m_desiredBatonOfficers && m_batonOfficerSpawnTimer <= 0.f) {
        sf::Vector2f spawnPos = playerPos + sf::Vector2f(static_cast<float>(rand() % 200 - 100), static_cast<float>(rand() % 200 - 100 + 250)); // Example offset
        spawnPolice(spawnPos, PoliceWeaponType::BATON);
        m_batonOfficerSpawnTimer = SPAWN_COOLDOWN_SECONDS;
    }

    // Spawn Pistol Officers
    if (countPoliceByType(PoliceWeaponType::PISTOL) < m_desiredPistolOfficers && m_pistolOfficerSpawnTimer <= 0.f) {
        sf::Vector2f spawnPos = playerPos + sf::Vector2f(static_cast<float>(rand() % 200 - 100 + 250), static_cast<float>(rand() % 200 - 100)); // Example offset
        spawnPolice(spawnPos, PoliceWeaponType::PISTOL);
        m_pistolOfficerSpawnTimer = SPAWN_COOLDOWN_SECONDS;
    }

    // Spawn Police Cars
    if (countPoliceCars() < m_desiredPoliceCars && m_policeCarSpawnTimer <= 0.f) {
        sf::Vector2f spawnPos = playerPos + sf::Vector2f(static_cast<float>(rand() % 300 - 150 - 300), static_cast<float>(rand() % 300 - 150)); // Example offset
        spawnPoliceCar(spawnPos);
        m_policeCarSpawnTimer = SPAWN_COOLDOWN_SECONDS;
    }

    // Spawn Police Helicopters
    if (countPoliceHelicopters() < m_desiredPoliceHelicopters && m_policeHelicopterSpawnTimer <= 0.f) {
        // Helicopters spawn higher and further, conceptually
        sf::Vector2f spawnPos = playerPos + sf::Vector2f(static_cast<float>(rand() % 400 - 200), static_cast<float>(rand() % 400 - 200 - 400));
        spawnPoliceHelicopter(spawnPos);
        m_policeHelicopterSpawnTimer = SPAWN_COOLDOWN_SECONDS * 2; // Slower spawn for helis
    }

    // Spawn Police Tanks
    if (countPoliceTanks() < m_desiredPoliceTanks && m_policeTankSpawnTimer <= 0.f) {
        sf::Vector2f spawnPos = playerPos + sf::Vector2f(static_cast<float>(rand() % 300 - 150 + 400), static_cast<float>(rand() % 300 - 150));
        spawnPoliceTank(spawnPos);
        m_policeTankSpawnTimer = SPAWN_COOLDOWN_SECONDS * 3; // Slower spawn for tanks
    }
}

// Spawn functions for new units
void PoliceManager::spawnPoliceHelicopter(const sf::Vector2f& position) {
    m_policeHelicopters.push_back(std::make_unique<PoliceHelicopter>(m_gameManager, position));
    // std::cout << "Spawned Police Helicopter at: (" << position.x << ", " << position.y << ")\n";
}

void PoliceManager::spawnPoliceTank(const sf::Vector2f& position) {
    m_policeTanks.push_back(std::make_unique<PoliceTank>(m_gameManager, position));
    // std::cout << "Spawned Police Tank at: (" << position.x << ", " << position.y << ")\n";
}


int PoliceManager::countPoliceByType(PoliceWeaponType type) {
    int count = 0;
    for (const auto& officer : m_policeOfficers) {
        if (officer->getWeaponType() == type) {
            count++;
        }
    }
    return count;
}

int PoliceManager::countPoliceCars() {
    return m_policeCars.size();
}

int PoliceManager::countPoliceHelicopters() {
    return m_policeHelicopters.size();
}

int PoliceManager::countPoliceTanks() {
    return m_policeTanks.size();
}

void PoliceManager::clearAllPolice() {
    m_policeOfficers.clear();
    m_policeCars.clear();
    m_policeHelicopters.clear(); // Clear new units
    m_policeTanks.clear();       // Clear new units
    // std::cout << "All police units cleared." << std::endl;
}


void PoliceManager::draw(sf::RenderTarget& window) {
    for (auto& unit : m_policeOfficers) {
        unit->draw(window);
    }
    for (auto& car : m_policeCars) {
        car->draw(window);
    }
    for (auto& heli : m_policeHelicopters) { // Draw new units
        heli->draw(window);
    }
    for (auto& tank : m_policeTanks) {       // Draw new units
        tank->draw(window);
    }
}

// Renamed from damageClosest for clarity
void PoliceManager::damageClosestOfficer(const sf::Vector2f& pos, int amount) {
    float minDist = std::numeric_limits<float>::max();
    Police* closestOfficer = nullptr;

    for (auto& unit : m_policeOfficers) {
        if (unit->isDead()) continue;
        float dist = std::hypot(unit->getPosition().x - pos.x, unit->getPosition().y - pos.y);
        if (dist < minDist) {
            minDist = dist;
            closestOfficer = unit.get();
        }
    }

    if (closestOfficer) {
        closestOfficer->takeDamage(amount);
    }
}

const std::vector<std::unique_ptr<PoliceCar>>& PoliceManager::getPoliceCars() const {
    return m_policeCars;
}

// This old spawning function might be deprecated or used for ambient 0-star police.
// For now, the wanted level system in managePolicePopulation is the primary spawner.
// void PoliceManager::spawnPoliceNearChunk(const sf::Vector2i& chunkCoord) {
//     float chunkSize = 256.f; 
//     sf::Vector2f basePos(chunkCoord.x * chunkSize, chunkCoord.y * chunkSize);
//     sf::Vector2f offset(rand() % 100 - 50, rand() % 100 - 50);
//     // Old spawnPolice didn't take weapon type. Defaulting to PISTOL for this example if it were used.
//     spawnPolice(basePos + offset, PoliceWeaponType::PISTOL); 
// }

// void PoliceManager::trySpawnRandomPoliceNear(const std::vector<sf::Vector2i>& activeChunks, const sf::Vector2f& playerPos) {
//     static float cooldown = 0.f; // This static var might cause issues if multiple PoliceManagers existed
//     cooldown -= 1.f / 60.f; // Assuming 60 FPS, effectively dt
// 
//     if (cooldown > 0.f) return;
//     cooldown = 7.f; 
// 
//     if (activeChunks.empty()) return;
//     if (rand() % 100 < 60) return; // 60% chance to skip
// 
//     const int chunkSize = CHUNK_SIZE; // CHUNK_SIZE needs to be defined, e.g., from Constants.h
//     sf::Vector2i chunk = activeChunks[rand() % activeChunks.size()];
//     sf::Vector2f pos = {
//         chunk.x * chunkSize + static_cast<float>(rand() % chunkSize),
//         chunk.y * chunkSize + static_cast<float>(rand() % chunkSize)
//     };
// 
//     float dist = std::hypot(playerPos.x - pos.x, playerPos.y - pos.y);
//     if (dist < 150.f || dist > 900.f) return; // Spawn within a certain range band
// 
//     // Defaulting to PISTOL for this random spawn, can be changed to BATON or random.
//     spawnPolice(pos, PoliceWeaponType::PISTOL); 
// }