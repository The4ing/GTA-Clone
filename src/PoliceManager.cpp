/**
#include "PoliceManager.h"
#include "GameManager.h"
#include "Player.h" // Include Player header
#include <limits>
#include <cmath>
#include <iostream> // For std::cout, std::cerr
#include <random>   // For std::mt19937, std::uniform_int_distribution
#include "PatrolZone.h" // Include for PatrolZone type
#include "RoadSegment.h"
#include "CollisionUtils.h"
#include "PathfindingGrid.h" // For isWalkable checks
#include <cmath> // For M_PI, cos, sin

// Initialize static members for pathfinding throttling
int s_pathfindingCallsThisFrame = 0;
const int S_MAX_PATHFINDING_CALLS_PER_FRAME = 10; // Increased slightly, can be tuned

PoliceManager::PoliceManager(GameManager& gameManager)
    : m_gameManager(gameManager),
    m_numSeeingPlayer(0),
    m_timePlayerNotSeen(0.0f),
    m_wantedReductionCooldownTimer(0.0f),
    m_targetBatonOfficers(0),
    m_targetPistolOfficers(0),
    m_targetPoliceCars(0),
    m_targetPoliceHelicopters(0),
    m_targetPoliceTanks(0),
    m_unitsToProcessPerFrame(2), // Process up to 2 units (spawn or despawn) per adjustment cycle
    m_populationAdjustmentCooldown(0.0f) {
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
    m_numSeeingPlayer = 0; // Reset counter each frame

    // Update spawn timers
    if (m_batonOfficerSpawnTimer > 0.f) m_batonOfficerSpawnTimer -= dt;
    if (m_pistolOfficerSpawnTimer > 0.f) m_pistolOfficerSpawnTimer -= dt;
    if (m_policeCarSpawnTimer > 0.f) m_policeCarSpawnTimer -= dt;
    if (m_policeHelicopterSpawnTimer > 0.f) m_policeHelicopterSpawnTimer -= dt; // Added
    if (m_policeTankSpawnTimer > 0.f) m_policeTankSpawnTimer -= dt;             // Added

    int wantedLevel = player.getWantedLevel();
    managePolicePopulation(wantedLevel, player.getPosition(), blockedPolygons);


    updatePoliceOfficers(dt, player, blockedPolygons); // Renamed from updatePoliceUnits
    updatePoliceCars(dt, player, blockedPolygons);
    updatePoliceHelicopters(dt, player, blockedPolygons); // Added
    updatePoliceTanks(dt, player, blockedPolygons);       // Added

    // Old random spawning logic (trySpawnRandomPoliceNear) might be disabled or integrated into managePolicePopulation for 0 stars.
    // if (wantedLevel == 0) {
    //    trySpawnRandomPoliceNear(m_gameManager.getActiveChunks(), player.getPosition()); // Assuming GameManager provides activeChunks
    // }

    // Wanted Level Reduction Logic
    if (m_wantedReductionCooldownTimer > 0.f) {
        m_wantedReductionCooldownTimer -= dt;
    }

    if (m_numSeeingPlayer == 0 && player.getWantedLevel() > 0) {
        m_timePlayerNotSeen += dt;
        if (m_timePlayerNotSeen >= TIME_TO_START_WANTED_REDUCTION && m_wantedReductionCooldownTimer <= 0.f) {
            player.setWantedLevel(player.getWantedLevel() - 1);
            std::cout << "Wanted level reduced to: " << player.getWantedLevel() << std::endl;
            m_timePlayerNotSeen = 0.0f; // Reset timer after reduction
            m_wantedReductionCooldownTimer = WANTED_REDUCTION_COOLDOWN_SECONDS; // Start cooldown
        }
    }
    else { // Player is seen or wanted level is 0
        m_timePlayerNotSeen = 0.0f;
    }
}

void PoliceManager::updatePoliceCars(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    for (auto& car : m_policeCars) {
        car->update(dt, player, blockedPolygons);
    }
    m_policeCars.erase(std::remove_if(m_policeCars.begin(), m_policeCars.end(),
        [](const std::unique_ptr<PoliceCar>& pc) { return pc->isDestroyed(); }),
        m_policeCars.end());

    for (const auto& car : m_policeCars) {
        if (!car->isDestroyed() && car->canSeePlayer(player, blockedPolygons)) {
            // Only count if car is hostile (not ambient and player not driving it)
            // Or if it's ambient and sees player committing a crime / high wanted level
            // For now, simpler: if it's a police car and sees player, it contributes.
            // More refined logic can be added if an ambient car seeing player shouldn't always count.
            if (!car->isAmbient() || player.getWantedLevel() > 0) { // Count if not ambient, OR if ambient and player is wanted
                if (car->getDriver() != &player) { // Don't count if player is driving this police car
                    m_numSeeingPlayer++;
                }
            }
        }
    }
}

// Renamed from updatePoliceUnits
void PoliceManager::updatePoliceOfficers(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    for (auto& unit : m_policeOfficers) {
        unit->update(dt, player, blockedPolygons);
    }
    m_policeOfficers.erase(std::remove_if(m_policeOfficers.begin(), m_policeOfficers.end(),
        [](const std::unique_ptr<Police>& p) { return p->isDead(); }),
        m_policeOfficers.end());

    for (const auto& unit : m_policeOfficers) {
        if (!unit->isDead() && unit->canSeePlayer(player, blockedPolygons)) {
            m_numSeeingPlayer++;
        }
    }
}

void PoliceManager::updatePoliceHelicopters(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    for (auto& heli : m_policeHelicopters) {
        heli->update(dt, player, blockedPolygons);
    }
    m_policeHelicopters.erase(std::remove_if(m_policeHelicopters.begin(), m_policeHelicopters.end(),
        [](const std::unique_ptr<PoliceHelicopter>& ph) { return ph->isDestroyed(); }),
        m_policeHelicopters.end());

    for (const auto& heli : m_policeHelicopters) {
        // Assuming PoliceHelicopter has canSeePlayer. If not, this needs to be added.
        // if (!heli->isDestroyed() && heli->canSeePlayer(player, blockedPolygons)) {
        //     m_numSeeingPlayer++;
        // }
    }
}

void PoliceManager::updatePoliceTanks(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    for (auto& tank : m_policeTanks) {
        tank->update(dt, player, blockedPolygons);
    }
    m_policeTanks.erase(std::remove_if(m_policeTanks.begin(), m_policeTanks.end(),
        [](const std::unique_ptr<PoliceTank>& pt) { return pt->isDestroyed(); }),
        m_policeTanks.end());

    for (const auto& tank : m_policeTanks) {
        // Assuming PoliceTank has canSeePlayer.
        // if (!tank->isDestroyed() && tank->canSeePlayer(player, blockedPolygons)) {
        //    m_numSeeingPlayer++;
        // }
    }
}


void PoliceManager::managePolicePopulation(int wantedLevel, const sf::Vector2f& playerPos, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons)
{
    m_desiredBatonOfficers = 0;
    m_desiredPistolOfficers = 0;
    m_desiredPoliceCars = 0;
    m_desiredPoliceHelicopters = 0;
    m_desiredPoliceTanks = 0;

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(1, 10);

    // קביעת מספר שוטרים רצוי לפי רמת ה־Wanted
    switch (wantedLevel) {
    case 1: m_desiredBatonOfficers = dist(rng); break;
    case 2: m_desiredBatonOfficers = dist(rng); m_desiredPistolOfficers = dist(rng); break;
    case 3: m_desiredBatonOfficers = dist(rng); m_desiredPistolOfficers = dist(rng); m_desiredPoliceCars = dist(rng); break;
    case 4: m_desiredBatonOfficers = dist(rng); m_desiredPistolOfficers = dist(rng); m_desiredPoliceCars = dist(rng); m_desiredPoliceHelicopters = dist(rng); break;
    case 5: m_desiredBatonOfficers = dist(rng); m_desiredPistolOfficers = dist(rng); m_desiredPoliceCars = dist(rng); m_desiredPoliceHelicopters = dist(rng); m_desiredPoliceTanks = dist(rng); break;
    default:
        if (wantedLevel > 5) {
            m_desiredBatonOfficers = 10;
            m_desiredPistolOfficers = 10;
            m_desiredPoliceCars = 10;
            m_desiredPoliceHelicopters = 10;
            m_desiredPoliceTanks = 10;
        }
        break;
    }

    // Spawn Baton Officers
    while (countPoliceByType(PoliceWeaponType::BATON) < m_desiredBatonOfficers && m_batonOfficerSpawnTimer <= 0.f) {
        for (int attempt = 0; attempt < 10; ++attempt) {
            sf::Vector2f spawnPos = playerPos + sf::Vector2f(rand() % 200 - 100, rand() % 200 - 100 + 250);
            if (!isInsideBlockedPolygon(spawnPos, blockedPolygons)) {
                spawnPolice(spawnPos, PoliceWeaponType::BATON);
                m_batonOfficerSpawnTimer = SPAWN_COOLDOWN_SECONDS;
                break;
            }
        }
        break;
    }

    // Spawn Pistol Officers
    while (countPoliceByType(PoliceWeaponType::PISTOL) < m_desiredPistolOfficers && m_pistolOfficerSpawnTimer <= 0.f) {
        for (int attempt = 0; attempt < 10; ++attempt) {
            sf::Vector2f spawnPos = playerPos + sf::Vector2f(rand() % 200 - 100 + 250, rand() % 200 - 100);
            if (!isInsideBlockedPolygon(spawnPos, blockedPolygons)) {
                spawnPolice(spawnPos, PoliceWeaponType::PISTOL);
                m_pistolOfficerSpawnTimer = SPAWN_COOLDOWN_SECONDS;
                break;
            }
        }
        break;
    }

    // Spawn Police Cars
    while (countPoliceCars() < m_desiredPoliceCars && m_policeCarSpawnTimer <= 0.f) {
        for (int attempt = 0; attempt < 10; ++attempt) {
            sf::Vector2f spawnPos = playerPos + sf::Vector2f(rand() % 300 - 150 - 300, rand() % 300 - 150);
            if (!isInsideBlockedPolygon(spawnPos, blockedPolygons)) {
                spawnPoliceCar(spawnPos);
                m_policeCarSpawnTimer = SPAWN_COOLDOWN_SECONDS;
                break;
            }
        }
        break;
    }

    // Spawn Helicopters (לא צריכים בדיקת פוליגון, הם באוויר)
    while (countPoliceHelicopters() < m_desiredPoliceHelicopters && m_policeHelicopterSpawnTimer <= 0.f) {
        sf::Vector2f spawnPos = playerPos + sf::Vector2f(rand() % 400 - 200, rand() % 400 - 200 - 400);
        spawnPoliceHelicopter(spawnPos);
        m_policeHelicopterSpawnTimer = SPAWN_COOLDOWN_SECONDS * 2;
        break;
    }

    // Spawn Tanks
    while (countPoliceTanks() < m_desiredPoliceTanks && m_policeTankSpawnTimer <= 0.f) {
        for (int attempt = 0; attempt < 10; ++attempt) {
            sf::Vector2f spawnPos = playerPos + sf::Vector2f(rand() % 300 - 150 + 400, rand() % 300 - 150);
            if (!isInsideBlockedPolygon(spawnPos, blockedPolygons)) {
                spawnPoliceTank(spawnPos);
                m_policeTankSpawnTimer = SPAWN_COOLDOWN_SECONDS * 3;
                break;
            }
        }
        break;
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

Police* PoliceManager::spawnPatrolOfficer(const sf::Vector2f& position, PoliceWeaponType weaponType, PatrolZone* zone) {
    auto newOfficer = std::make_unique<Police>(m_gameManager, weaponType);
    Police* officerPtr = newOfficer.get(); // Get raw pointer before moving
    newOfficer->setPosition(position);
    newOfficer->setPatrolZone(zone);
    // Patrol officers should start in a non-aggressive, patrolling state.
    // (Their internal AI will handle this based on having a zone and current wanted level)
    m_policeOfficers.push_back(std::move(newOfficer));
    return officerPtr;
}

PoliceCar* PoliceManager::spawnPatrolCar(const sf::Vector2f& position, PatrolZone* zone) {
    auto newCar = std::make_unique<PoliceCar>(m_gameManager, position);
    PoliceCar* carPtr = newCar.get(); // Get raw pointer before moving
    newCar->setPatrolZone(zone);
    newCar->setIsAmbient(true); // Patrol cars start as "ambient" within their zone
    newCar->m_playerCausedWantedIncrease = false; // Ensure this is reset
    // Additional setup like finding nearest road and setting initial direction might be needed here or in PoliceCar when a zone is set.
    m_policeCars.push_back(std::move(newCar));
    return carPtr;
}

bool PoliceManager::isInsideBlockedPolygon(const sf::Vector2f& point, const std::vector<std::vector<sf::Vector2f>>& polygons) {
    return CollisionUtils::isInsideBlockedPolygon(point, polygons);
}

*/

#include "PoliceManager.h"
#include "GameManager.h"
#include "Player.h" // Include Player header
#include <limits>
#include <cmath>
#include <iostream> // For std::cout, std::cerr
#include <random>   // For std::mt19937, std::uniform_int_distribution
#include "PatrolZone.h" // Include for PatrolZone type
#include "RoadSegment.h"
#include "CollisionUtils.h"

// Initialize static members for pathfinding throttling
int s_pathfindingCallsThisFrame = 0;
const int S_MAX_PATHFINDING_CALLS_PER_FRAME = 10; // Increased slightly, can be tuned

PoliceManager::PoliceManager(GameManager& gameManager)
    : m_gameManager(gameManager),
    m_numSeeingPlayer(0),
    m_timePlayerNotSeen(0.0f),
    m_wantedReductionCooldownTimer(0.0f) {
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
void PoliceManager::spawnPolice(const sf::Vector2f& position, PoliceWeaponType weaponType, bool isStatic) {
    auto newOfficer = std::make_unique<Police>(m_gameManager, weaponType);
    newOfficer->setPosition(position);
    newOfficer->setIsStatic(isStatic);
    if (isStatic) {
        m_staticPoliceOfficers.push_back(std::move(newOfficer));
        // std::cout << "Spawned STATIC " << (weaponType == PoliceWeaponType::BATON ? "Baton Police" : "Pistol Police")
        //           << " at: (" << position.x << ", " << position.y << ")\n";
    } else {
        m_policeOfficers.push_back(std::move(newOfficer));
        // std::cout << "Spawned DYNAMIC " << (weaponType == PoliceWeaponType::BATON ? "Baton Police" : "Pistol Police")
        //           << " at: (" << position.x << ", " << position.y << ")\n";
    }
}

// Spawn a police car
void PoliceManager::spawnPoliceCar(const sf::Vector2f& position, bool isStatic) {
    auto newCar = std::make_unique<PoliceCar>(m_gameManager, position);
    newCar->setPosition(position); // Make sure position is set
    newCar->setIsAmbient(false); // Aggressive by default unless specified otherwise
    newCar->setIsStatic(isStatic);
    if (isStatic) {
        m_staticPoliceCars.push_back(std::move(newCar));
        // std::cout << "Spawned STATIC AGGRESSIVE Police Car at: (" << position.x << ", " << position.y << ")\n";
    } else {
        m_policeCars.push_back(std::move(newCar));
        // std::cout << "Spawned DYNAMIC AGGRESSIVE Police Car at: (" << position.x << ", " << position.y << ")\n";
    }
}

void PoliceManager::spawnStaticPoliceUnits(const sf::FloatRect& mapBounds, float gridSize, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    std::mt19937 rng(std::random_device{}()); // For randomizing position within a grid cell

    for (float x = mapBounds.left; x < mapBounds.left + mapBounds.width; x += gridSize) {
        for (float y = mapBounds.top; y < mapBounds.top + mapBounds.height; y += gridSize) {
            // Attempt to spawn one baton police, one pistol police, and one police car per grid cell
            // Try a few random positions within the cell to find an unblocked spot

            // Baton Police
            for (int attempt = 0; attempt < 5; ++attempt) {
                std::uniform_real_distribution<float> distX(x, std::min(x + gridSize, mapBounds.left + mapBounds.width));
                std::uniform_real_distribution<float> distY(y, std::min(y + gridSize, mapBounds.top + mapBounds.height));
                sf::Vector2f spawnPos(distX(rng), distY(rng));
                if (!isInsideBlockedPolygon(spawnPos, blockedPolygons)) {
                    spawnPolice(spawnPos, PoliceWeaponType::BATON, true);
                    break;
                }
            }

            // Pistol Police
            for (int attempt = 0; attempt < 5; ++attempt) {
                std::uniform_real_distribution<float> distX(x, std::min(x + gridSize, mapBounds.left + mapBounds.width));
                std::uniform_real_distribution<float> distY(y, std::min(y + gridSize, mapBounds.top + mapBounds.height));
                sf::Vector2f spawnPos(distX(rng), distY(rng));
                if (!isInsideBlockedPolygon(spawnPos, blockedPolygons)) {
                    spawnPolice(spawnPos, PoliceWeaponType::PISTOL, true);
                    break;
                }
            }

            // Police Car
            for (int attempt = 0; attempt < 5; ++attempt) {
                std::uniform_real_distribution<float> distX(x, std::min(x + gridSize, mapBounds.left + mapBounds.width));
                std::uniform_real_distribution<float> distY(y, std::min(y + gridSize, mapBounds.top + mapBounds.height));
                sf::Vector2f spawnPos(distX(rng), distY(rng));
                if (!isInsideBlockedPolygon(spawnPos, blockedPolygons)) {
                    spawnPoliceCar(spawnPos, true);
                    break;
                }
            }
        }
    }
    std::cout << "Spawned " << m_staticPoliceOfficers.size() << " static police officers and "
              << m_staticPoliceCars.size() << " static police cars." << std::endl;
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
    m_numSeeingPlayer = 0; // Reset counter each frame

    // Update spawn timers
    if (m_batonOfficerSpawnTimer > 0.f) m_batonOfficerSpawnTimer -= dt;
    if (m_pistolOfficerSpawnTimer > 0.f) m_pistolOfficerSpawnTimer -= dt;
    if (m_policeCarSpawnTimer > 0.f) m_policeCarSpawnTimer -= dt;
    if (m_policeHelicopterSpawnTimer > 0.f) m_policeHelicopterSpawnTimer -= dt; // Added
    if (m_policeTankSpawnTimer > 0.f) m_policeTankSpawnTimer -= dt;             // Added

    int wantedLevel = player.getWantedLevel();
    // We need the game view to determine off-screen spawn locations
    // This assumes GameManager has a method like getView() or getGameView()
    m_populationAdjustmentCooldown -= dt;
    if (m_populationAdjustmentCooldown <= 0.f) {
        managePolicePopulation(wantedLevel, player.getPosition(), m_gameManager.getGameView(), blockedPolygons);
        m_populationAdjustmentCooldown = POPULATION_ADJUST_INTERVAL; // Reset cooldown
    }

    updatePoliceOfficers(dt, player, blockedPolygons);
    updatePoliceCars(dt, player, blockedPolygons);
    updatePoliceHelicopters(dt, player, blockedPolygons);
    updatePoliceTanks(dt, player, blockedPolygons);

    // Update static units as well
    for (auto& unit : m_staticPoliceOfficers) {
        unit->update(dt, player, blockedPolygons);
        if (!unit->isDead() && unit->canSeePlayer(player, blockedPolygons)) {
            m_numSeeingPlayer++; // Static units also contribute to being seen
        }
    }
    m_staticPoliceOfficers.erase(std::remove_if(m_staticPoliceOfficers.begin(), m_staticPoliceOfficers.end(),
        [](const std::unique_ptr<Police>& p) { return p->needsCleanup; }),
        m_staticPoliceOfficers.end());

    for (auto& car : m_staticPoliceCars) {
        car->update(dt, player, blockedPolygons);
        if (!car->isDestroyed() && car->canSeePlayer(player, blockedPolygons)) {
            if (car->getDriver() != &player) {
                 m_numSeeingPlayer++; // Static cars also contribute
            }
        }
    }
    m_staticPoliceCars.erase(std::remove_if(m_staticPoliceCars.begin(), m_staticPoliceCars.end(),
        [](const std::unique_ptr<PoliceCar>& pc) { return pc->isDestroyed(); }),
        m_staticPoliceCars.end());

    // Old random spawning logic (trySpawnRandomPoliceNear) might be disabled or integrated into managePolicePopulation for 0 stars.
    // if (wantedLevel == 0) {
    //    trySpawnRandomPoliceNear(m_gameManager.getActiveChunks(), player.getPosition()); // Assuming GameManager provides activeChunks
    // }

    // Wanted Level Reduction Logic
    if (m_wantedReductionCooldownTimer > 0.f) {
        m_wantedReductionCooldownTimer -= dt;
    }

    if (m_numSeeingPlayer == 0 && player.getWantedLevel() > 0) {
        m_timePlayerNotSeen += dt;
        if (m_timePlayerNotSeen >= TIME_TO_START_WANTED_REDUCTION && m_wantedReductionCooldownTimer <= 0.f) {
            player.setWantedLevel(player.getWantedLevel() - 1);
            std::cout << "Wanted level reduced to: " << player.getWantedLevel() << std::endl;
            m_timePlayerNotSeen = 0.0f; // Reset timer after reduction
            m_wantedReductionCooldownTimer = WANTED_REDUCTION_COOLDOWN_SECONDS; // Start cooldown
        }
    }
    else { // Player is seen or wanted level is 0
        m_timePlayerNotSeen = 0.0f;
    }
}

void PoliceManager::updatePoliceCars(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    for (auto& car : m_policeCars) {
        car->update(dt, player, blockedPolygons);
    }
    m_policeCars.erase(std::remove_if(m_policeCars.begin(), m_policeCars.end(),
        [](const std::unique_ptr<PoliceCar>& pc) { return pc->isDestroyed() || pc->needsCleanup; }), // Also remove if needsCleanup
        m_policeCars.end());

    for (const auto& car : m_policeCars) {
        if (!car->isDestroyed() && !car->isRetreating() && car->canSeePlayer(player, blockedPolygons)) { // Don't count retreating cars
            if (!car->isAmbient() || player.getWantedLevel() > 0) {
                if (car->getDriver() != &player) {
                    m_numSeeingPlayer++;
                }
            }
        }
    }
}

// Renamed from updatePoliceUnits
void PoliceManager::updatePoliceOfficers(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    for (auto& unit : m_policeOfficers) {
        unit->update(dt, player, blockedPolygons);
    }
    m_policeOfficers.erase(std::remove_if(m_policeOfficers.begin(), m_policeOfficers.end(),
        [](const std::unique_ptr<Police>& p) { return p->needsCleanup; }),
        m_policeOfficers.end());

    for (const auto& unit : m_policeOfficers) {
        if (!unit->isDead() && !unit->isRetreating() && unit->canSeePlayer(player, blockedPolygons)) { // Don't count retreating units as seeing player
            m_numSeeingPlayer++;
        }
    }
}

void PoliceManager::updatePoliceHelicopters(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    for (auto& heli : m_policeHelicopters) {
        heli->update(dt, player, blockedPolygons);
    }
    m_policeHelicopters.erase(std::remove_if(m_policeHelicopters.begin(), m_policeHelicopters.end(),
        [](const std::unique_ptr<PoliceHelicopter>& ph) { return ph->isDestroyed(); }),
        m_policeHelicopters.end());

    for (const auto& heli : m_policeHelicopters) {
        // Assuming PoliceHelicopter has canSeePlayer. If not, this needs to be added.
        // if (!heli->isDestroyed() && heli->canSeePlayer(player, blockedPolygons)) {
        //     m_numSeeingPlayer++;
        // }
    }
}

void PoliceManager::updatePoliceTanks(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    for (auto& tank : m_policeTanks) {
        tank->update(dt, player, blockedPolygons);
    }
    m_policeTanks.erase(std::remove_if(m_policeTanks.begin(), m_policeTanks.end(),
        [](const std::unique_ptr<PoliceTank>& pt) { return pt->isDestroyed(); }),
        m_policeTanks.end());

    for (const auto& tank : m_policeTanks) {
        // Assuming PoliceTank has canSeePlayer.
        // if (!tank->isDestroyed() && tank->canSeePlayer(player, blockedPolygons)) {
        //    m_numSeeingPlayer++;
        // }
    }
}


sf::Vector2f findOffScreenSpawnPosition(const sf::View& gameView, const sf::Vector2f& playerPos, float minSpawnDistFromPlayer, float minSpawnDistFromScreenEdge, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons, GameManager& gameManager) {
    sf::FloatRect viewRect(gameView.getCenter() - gameView.getSize() / 2.f, gameView.getSize());
    std::mt19937 rng(std::random_device{}());
    PathfindingGrid* pfGrid = gameManager.getPathfindingGrid(); // Get the grid

    for (int attempts = 0; attempts < 20; ++attempts) { // Try multiple times to find a spot
        float angle = std::uniform_real_distribution<float>(0, 2 * static_cast<float>(M_PI))(rng);
        // Spawn further away than just the screen edge
        float distance_from_player = std::uniform_real_distribution<float>(minSpawnDistFromPlayer, minSpawnDistFromPlayer + 200.f)(rng);

        sf::Vector2f spawnPos;
        spawnPos.x = playerPos.x + distance_from_player * std::cos(angle);
        spawnPos.y = playerPos.y + distance_from_player * std::sin(angle);

        // Check if it's outside the view rectangle + a buffer
        sf::FloatRect extendedViewRect = viewRect;
        extendedViewRect.left -= minSpawnDistFromScreenEdge;
        extendedViewRect.top -= minSpawnDistFromScreenEdge;
        extendedViewRect.width += 2 * minSpawnDistFromScreenEdge;
        extendedViewRect.height += 2 * minSpawnDistFromScreenEdge;

        if (!extendedViewRect.contains(spawnPos)) { // It's sufficiently off-screen
             // Check map boundaries
            if (spawnPos.x < 0 || spawnPos.x >= MAP_WIDTH || spawnPos.y < 0 || spawnPos.y >= MAP_HEIGHT) {
                continue; // Outside map
            }

            // OPTIMIZATION: Use PathfindingGrid for initial check
            if (pfGrid && !pfGrid->isWalkable(spawnPos)) {
                continue; // Position is not walkable according to the pathfinding grid
            }

            // Original, more expensive check (can be kept for finer detail or removed if pfGrid is sufficient)
            // For now, let's assume pfGrid is good enough to replace the direct polygon check for spawning.
            // If issues arise with units spawning in visually awkward (but pathable) spots, this can be re-enabled.
            /*
            bool blockedByPolygon = false;
            for (const auto& poly : blockedPolygons) { // This is the expensive part
                if (CollisionUtils::pointInPolygon(spawnPos, poly)) {
                    blockedByPolygon = true;
                    break;
                }
            }
            if (blockedByPolygon) {
                continue;
            }
            */

            // If we reach here, the spot is considered valid
            return spawnPos;
        }
    }
    // Fallback: if no good off-screen spot found, spawn near player but try to be outside immediate view
    float fallbackAngle = std::uniform_real_distribution<float>(0, 2 * static_cast<float>(M_PI))(rng);
    sf::Vector2f fallbackPos = playerPos + sf::Vector2f(std::cos(fallbackAngle) * (viewRect.width / 2.f + minSpawnDistFromScreenEdge), // Added .f for literals
                                   std::sin(fallbackAngle) * (viewRect.height / 2.f + minSpawnDistFromScreenEdge)); // Added .f for literals

    // Ensure fallback is within map bounds and walkable if possible
    fallbackPos.x = std::max(0.f, std::min(fallbackPos.x, (float)MAP_WIDTH -1.f));
    fallbackPos.y = std::max(0.f, std::min(fallbackPos.y, (float)MAP_HEIGHT-1.f));

    if (pfGrid && !pfGrid->isWalkable(fallbackPos)) {
        // If even fallback is not walkable, this is tricky.
        // Potentially log this, or try a slightly different fallback.
        // For now, return the potentially unwalkable fallback. The unit's AI might struggle.
        // std::cerr << "Warning: Fallback spawn position (" << fallbackPos.x << ", " << fallbackPos.y << ") is not walkable." << std::endl;
    }
    return fallbackPos;
}


void PoliceManager::managePolicePopulation(int wantedLevel, const sf::Vector2f& playerPos, const sf::View& gameView, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons)
{
    // --- Part A: Determine Target Population ---
    // (The existing switch statement to set m_desired* variables will now set m_target* variables)
    switch (wantedLevel) {
    case 0:
        m_targetBatonOfficers = 0; m_targetPistolOfficers = 0; m_targetPoliceCars = 0;
        m_targetPoliceHelicopters = 0; m_targetPoliceTanks = 0;
        break;
    case 1:
        m_targetBatonOfficers = 2; m_targetPistolOfficers = 0; m_targetPoliceCars = 0;
        m_targetPoliceHelicopters = 0; m_targetPoliceTanks = 0;
        break;
    case 2:
        m_targetBatonOfficers = 3; m_targetPistolOfficers = 2; m_targetPoliceCars = 1;
        m_targetPoliceHelicopters = 0; m_targetPoliceTanks = 0;
        break;
    case 3:
        m_targetBatonOfficers = 2; m_targetPistolOfficers = 3; m_targetPoliceCars = 2;
        m_targetPoliceHelicopters = 0; m_targetPoliceTanks = 0;
        break;
    case 4:
        m_targetPistolOfficers = 4; m_targetPoliceCars = 3; m_targetPoliceHelicopters = 1;
        m_targetBatonOfficers = 1; m_targetPoliceTanks = 0;
        break;
    case 5:
        m_targetPistolOfficers = 5; m_targetPoliceCars = 3; m_targetPoliceHelicopters = 2;
        m_targetPoliceTanks = 1; m_targetBatonOfficers = 0;
        break;
    default: // For wanted levels > 5
        m_targetPistolOfficers = 6; m_targetPoliceCars = 4; m_targetPoliceHelicopters = 2;
        m_targetPoliceTanks = 2; m_targetBatonOfficers = 0;
        break;
    }

    // --- Part B: Process Spawns/Despawns (staggered) ---
    int processedThisCycle = 0;

    const float minSpawnDistFromPlayer = std::max(gameView.getSize().x, gameView.getSize().y) * 0.6f;
    const float minSpawnDistFromScreenEdge = 50.f;

    // --- Spawning Logic (Dynamic Units Only) ---
    // Baton Officers
    int currentDynamicBatonOfficers = countDynamicPoliceByType(PoliceWeaponType::BATON);
    if (currentDynamicBatonOfficers < m_targetBatonOfficers && m_batonOfficerSpawnTimer <= 0.f && processedThisCycle < m_unitsToProcessPerFrame) {
        sf::Vector2f spawnPos = findOffScreenSpawnPosition(gameView, playerPos, minSpawnDistFromPlayer, minSpawnDistFromScreenEdge, blockedPolygons, m_gameManager);
        spawnPolice(spawnPos, PoliceWeaponType::BATON, false);
        m_batonOfficerSpawnTimer = SPAWN_COOLDOWN_SECONDS;
        processedThisCycle++;
    }

    // Pistol Officers
    int currentDynamicPistolOfficers = countDynamicPoliceByType(PoliceWeaponType::PISTOL);
    if (currentDynamicPistolOfficers < m_targetPistolOfficers && m_pistolOfficerSpawnTimer <= 0.f && processedThisCycle < m_unitsToProcessPerFrame) {
        sf::Vector2f spawnPos = findOffScreenSpawnPosition(gameView, playerPos, minSpawnDistFromPlayer, minSpawnDistFromScreenEdge, blockedPolygons, m_gameManager);
        spawnPolice(spawnPos, PoliceWeaponType::PISTOL, false);
        m_pistolOfficerSpawnTimer = SPAWN_COOLDOWN_SECONDS;
        processedThisCycle++;
    }

    // Police Cars
    int currentDynamicPoliceCars = countDynamicPoliceCars();
    if (currentDynamicPoliceCars < m_targetPoliceCars && m_policeCarSpawnTimer <= 0.f && processedThisCycle < m_unitsToProcessPerFrame) {
        sf::Vector2f spawnPos = findOffScreenSpawnPosition(gameView, playerPos, minSpawnDistFromPlayer, minSpawnDistFromScreenEdge, blockedPolygons, m_gameManager);
        spawnPoliceCar(spawnPos, false);
        m_policeCarSpawnTimer = SPAWN_COOLDOWN_SECONDS;
        processedThisCycle++;
    }

    // Helicopters
    if (countPoliceHelicopters() < m_targetPoliceHelicopters && m_policeHelicopterSpawnTimer <= 0.f && processedThisCycle < m_unitsToProcessPerFrame) {
        sf::Vector2f spawnPos = findOffScreenSpawnPosition(gameView, playerPos, minSpawnDistFromPlayer + 100.f, minSpawnDistFromScreenEdge + 50.f, {}, m_gameManager);
        spawnPoliceHelicopter(spawnPos);
        m_policeHelicopterSpawnTimer = SPAWN_COOLDOWN_SECONDS * 2;
        processedThisCycle++;
    }

    // Tanks
    if (countPoliceTanks() < m_targetPoliceTanks && m_policeTankSpawnTimer <= 0.f && processedThisCycle < m_unitsToProcessPerFrame) {
        sf::Vector2f spawnPos = findOffScreenSpawnPosition(gameView, playerPos, minSpawnDistFromPlayer + 50.f, minSpawnDistFromScreenEdge, blockedPolygons, m_gameManager);
        spawnPoliceTank(spawnPos);
        m_policeTankSpawnTimer = SPAWN_COOLDOWN_SECONDS * 3;
        processedThisCycle++;
    }

    // --- Despawning Logic (Retreat for Dynamic Units Only) ---
    if (processedThisCycle >= m_unitsToProcessPerFrame) {
        return; // Max units processed for this cycle
    }

    // Baton Officers to Retreat
    currentDynamicBatonOfficers = countDynamicPoliceByType(PoliceWeaponType::BATON); // Recount in case one was just spawned
    if (currentDynamicBatonOfficers > m_targetBatonOfficers && processedThisCycle < m_unitsToProcessPerFrame) {
        for (auto& officer : m_policeOfficers) {
            if (!officer->isStatic() && officer->getWeaponType() == PoliceWeaponType::BATON && !officer->isRetreating()) {
                sf::Vector2f retreatTarget = findOffScreenSpawnPosition(gameView, playerPos, minSpawnDistFromPlayer, minSpawnDistFromScreenEdge, blockedPolygons, m_gameManager);
                officer->startRetreating(retreatTarget);
                processedThisCycle++;
                break;
            }
        }
    }

    // Pistol Officers to Retreat
    if (processedThisCycle >= m_unitsToProcessPerFrame) return;
    currentDynamicPistolOfficers = countDynamicPoliceByType(PoliceWeaponType::PISTOL); // Recount
    if (currentDynamicPistolOfficers > m_targetPistolOfficers && processedThisCycle < m_unitsToProcessPerFrame) {
        for (auto& officer : m_policeOfficers) {
            if (!officer->isStatic() && officer->getWeaponType() == PoliceWeaponType::PISTOL && !officer->isRetreating()) {
                sf::Vector2f retreatTarget = findOffScreenSpawnPosition(gameView, playerPos, minSpawnDistFromPlayer, minSpawnDistFromScreenEdge, blockedPolygons, m_gameManager);
                officer->startRetreating(retreatTarget);
                processedThisCycle++;
                break;
            }
        }
    }

    // Police Cars to Retreat
    if (processedThisCycle >= m_unitsToProcessPerFrame) return;
    currentDynamicPoliceCars = countDynamicPoliceCars(); // Recount
    if (currentDynamicPoliceCars > m_targetPoliceCars && processedThisCycle < m_unitsToProcessPerFrame) {
        for (auto& car : m_policeCars) {
            if (!car->isStatic() && !car->isRetreating() && !car->isAmbient()) {
                sf::Vector2f retreatTarget = findOffScreenSpawnPosition(gameView, playerPos, minSpawnDistFromPlayer, minSpawnDistFromScreenEdge, blockedPolygons, m_gameManager);
                car->startRetreating(retreatTarget);
                processedThisCycle++;
                break;
            }
        }
    }

    // Note: Helicopters and Tanks currently don't have a specific "retreat" command here.
    // They are typically managed by being destroyed or flying off-screen leading to cleanup.
    // If active despawning is needed for them when counts are too high, it would be more complex:
    // e.g., find a non-critical, far-away unit and remove it.
    // For now, their population decreases primarily through attrition or existing cleanup mechanisms.
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


int PoliceManager::countPoliceByType(PoliceWeaponType type) { // Counts ALL officers of a type
    int count = 0;
    for (const auto& officer : m_policeOfficers) { // Dynamic
        if (officer->getWeaponType() == type) {
            count++;
        }
    }
    for (const auto& officer : m_staticPoliceOfficers) { // Static
        if (officer->getWeaponType() == type) {
            count++;
        }
    }
    return count;
}

int PoliceManager::countDynamicPoliceByType(PoliceWeaponType type) { // Counts DYNAMIC officers of a type
    int count = 0;
    for (const auto& officer : m_policeOfficers) {
        if (!officer->isStatic() && officer->getWeaponType() == type) {
            count++;
        }
    }
    return count;
}


int PoliceManager::countPoliceCars() { // Counts ALL cars
    return m_policeCars.size() + m_staticPoliceCars.size();
}

int PoliceManager::countDynamicPoliceCars() { // Counts DYNAMIC cars
    int count = 0;
    for (const auto& car : m_policeCars) {
        if (!car->isStatic()) {
            count++;
        }
    }
    return count;
}

int PoliceManager::countPoliceHelicopters() {
    return m_policeHelicopters.size();
}

int PoliceManager::countPoliceTanks() {
    return m_policeTanks.size();
}

void PoliceManager::clearAllPolice() {
    m_policeOfficers.clear(); // Dynamic
    m_policeCars.clear();     // Dynamic
    m_policeHelicopters.clear();
    m_policeTanks.clear();
    m_staticPoliceOfficers.clear(); // Static
    m_staticPoliceCars.clear();     // Static
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

    // Draw static units
    for (auto& unit : m_staticPoliceOfficers) {
        unit->draw(window);
    }
    for (auto& car : m_staticPoliceCars) {
        car->draw(window);
    }
}

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

const std::vector<std::unique_ptr<Police>>& PoliceManager::getPoliceOfficers() const {
    return m_policeOfficers;
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

Police* PoliceManager::spawnPatrolOfficer(const sf::Vector2f& position, PoliceWeaponType weaponType, PatrolZone* zone) {
    auto newOfficer = std::make_unique<Police>(m_gameManager, weaponType);
    Police* officerPtr = newOfficer.get(); // Get raw pointer before moving
    newOfficer->setPosition(position);
    newOfficer->setPatrolZone(zone);
    // Patrol officers should start in a non-aggressive, patrolling state.
    // (Their internal AI will handle this based on having a zone and current wanted level)
    m_policeOfficers.push_back(std::move(newOfficer));
    return officerPtr;
}

PoliceCar* PoliceManager::spawnPatrolCar(const sf::Vector2f& position, PatrolZone* zone) {
    auto newCar = std::make_unique<PoliceCar>(m_gameManager, position);
    PoliceCar* carPtr = newCar.get(); // Get raw pointer before moving
    newCar->setPatrolZone(zone);
    newCar->setIsAmbient(true); // Patrol cars start as "ambient" within their zone
    newCar->m_playerCausedWantedIncrease = false; // Ensure this is reset
    // Additional setup like finding nearest road and setting initial direction might be needed here or in PoliceCar when a zone is set.
    m_policeCars.push_back(std::move(newCar));
    return carPtr;
}

bool PoliceManager::isInsideBlockedPolygon(const sf::Vector2f& point, const std::vector<std::vector<sf::Vector2f>>& polygons) {
    return CollisionUtils::isInsideBlockedPolygon(point, polygons);
}