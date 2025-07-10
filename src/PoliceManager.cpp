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
#include "CarManager.h"

static float distanceSquared(const sf::Vector2f & a, const sf::Vector2f & b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy;
}

// Initialize static members for pathfinding throttling
int s_pathfindingCallsThisFrame = 0;
const int S_MAX_PATHFINDING_CALLS_PER_FRAME = 10; // Increased slightly, can be tuned
// When the wanted level changes a large number of units might try to retreat
// at once.  Calculating retreat paths for all of them in the same frame causes
// noticeable stalls.  Limit how many units are processed per frame.
const int MAX_RETREATS_PER_FRAME = 3;

static bool isValidSpawnPosition(const sf::Vector2f& pos, const GameManager& gameManager) {
    if (pos.x < 0 || pos.x >= MAP_WIDTH || pos.y < 0 || pos.y >= MAP_HEIGHT)
        return false;
    if (gameManager.isPositionBlocked(pos))
        return false;
    if (auto grid = gameManager.getPathfindingGrid()) {
        sf::Vector2i gp = grid->worldToGrid(pos);
        if (!grid->isCellWalkable(gp.x, gp.y))
            return false;
    }
    return true;
}

PoliceManager::PoliceManager(GameManager& gameManager)
    : m_gameManager(gameManager),
    m_numSeeingPlayer(0),
    m_timePlayerNotSeen(0.0f),
    m_wantedReductionCooldownTimer(0.0f),
    m_prevWantedLevel(0) {
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
    }
    else {
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
    }
    else {
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
                if (isValidSpawnPosition(spawnPos, m_gameManager)) {
                    spawnPolice(spawnPos, PoliceWeaponType::BATON, true);
                    break;
                }
            }

            // Pistol Police
            for (int attempt = 0; attempt < 5; ++attempt) {
                std::uniform_real_distribution<float> distX(x, std::min(x + gridSize, mapBounds.left + mapBounds.width));
                std::uniform_real_distribution<float> distY(y, std::min(y + gridSize, mapBounds.top + mapBounds.height));
                sf::Vector2f spawnPos(distX(rng), distY(rng));
                if (isValidSpawnPosition(spawnPos, m_gameManager)) {
                    spawnPolice(spawnPos, PoliceWeaponType::PISTOL, true);
                    break;
                }
            }

            // Police Car
            for (int attempt = 0; attempt < 5; ++attempt) {
                std::uniform_real_distribution<float> distX(x, std::min(x + gridSize, mapBounds.left + mapBounds.width));
                std::uniform_real_distribution<float> distY(y, std::min(y + gridSize, mapBounds.top + mapBounds.height));
                sf::Vector2f spawnPos(distX(rng), distY(rng));
                if (isValidSpawnPosition(spawnPos, m_gameManager)) {
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
    if (wantedLevel < m_prevWantedLevel) {
        retreatAllCars(player, blockedPolygons);
    }
    m_prevWantedLevel = wantedLevel;
    // We need the game view to determine off-screen spawn locations
    // This assumes GameManager has a method like getView() or getGameView()
    // For now, I'll pass it through update. This might need a cleaner solution.
    managePolicePopulation(wantedLevel, player.getPosition(), m_gameManager.getGameView(), blockedPolygons);


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
            SoundManager::getInstance().playSound("notWanted");
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
        if (car->hasOfficerInside() && car->readyForOfficerExit()) {
            float rot = (car->getSprite().getRotation() - 90.f) * static_cast<float>(M_PI) / 180.f;
            sf::Vector2f side(std::cos(rot + M_PI / 2.f), std::sin(rot + M_PI / 2.f));
            sf::Vector2f exitPos = car->getPosition() + side * 20.f;
            spawnPolice(exitPos, PoliceWeaponType::PISTOL);
            car->setOfficerInside(false);
            car->setIsStatic(true);
            car->stop();
            car->clearOfficerExitRequest();
        }
    }
    m_policeCars.erase(std::remove_if(m_policeCars.begin(), m_policeCars.end(),
        [](const std::unique_ptr<PoliceCar>& pc) { return pc->isDestroyed() || pc->needsCleanup; }), m_policeCars.end());

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
    const sf::View& view = m_gameManager.getGameView();
    const sf::Vector2f viewCenter = view.getCenter();
    const float viewRadius = std::max(view.getSize().x, view.getSize().y) * 0.5f;
    const float cleanupDistance = viewRadius + 400.f; // מרחק שמעבר לו נחשיב את המסוק כרחוק

    for (auto& heli : m_policeHelicopters) {
        if (player.getWantedLevel() < 4) {
            // התחלת נסיגה אם עוד לא התחיל
            if (!heli->isRetreating()) {
                sf::Vector2f dir = heli->getPosition() - viewCenter;
                float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (len == 0.f) dir = { 1.f, 0.f }; else dir /= len;
                sf::Vector2f target = viewCenter + dir * (viewRadius + 300.f);
                heli->startRetreating(target);
            }

            // בדוק אם התרחק מספיק — אם כן, סמן למחיקה
            sf::Vector2f distVec = heli->getPosition() - viewCenter;
            float dist = std::sqrt(distVec.x * distVec.x + distVec.y * distVec.y);
            if (dist > cleanupDistance) {
                heli->needsCleanup = true;
            }
        }

        heli->update(dt, player, blockedPolygons);
    }

    m_policeHelicopters.erase(std::remove_if(m_policeHelicopters.begin(), m_policeHelicopters.end(),
        [](const std::unique_ptr<PoliceHelicopter>& ph) {
            return ph->isDestroyed() || ph->needsCleanup;
        }),
        m_policeHelicopters.end());

    m_numSeeingPlayer = 0; // reset count
    for (const auto& heli : m_policeHelicopters) {
        if (!heli->isDestroyed() && heli->canSeePlayer(player, blockedPolygons)) {
            m_numSeeingPlayer++;
        }
    }
}


void PoliceManager::updatePoliceTanks(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    for (auto& tank : m_policeTanks) {
        tank->update(dt, player, blockedPolygons);
    }
    m_policeTanks.erase(std::remove_if(m_policeTanks.begin(), m_policeTanks.end(),
        [](const std::unique_ptr<PoliceTank>& pt) { return pt->isDestroyed() || pt->isReadyForCleanup(); }),
        m_policeTanks.end());

    for (const auto& tank : m_policeTanks) {
        if (!tank->isDestroyed() && tank->canSeePlayer(player, blockedPolygons)) {
            m_numSeeingPlayer++;
        }
    }
}


sf::Vector2f findOffScreenSpawnPosition(const sf::View& gameView, const sf::Vector2f& playerPos, float minSpawnDistFromPlayer, float minSpawnDistFromScreenEdge, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons, GameManager& gameManager) {
    sf::FloatRect viewRect(gameView.getCenter() - gameView.getSize() / 2.f, gameView.getSize());
    std::mt19937 rng(std::random_device{}());

    for (int attempts = 0; attempts < 20; ++attempts) { // Try multiple times to find a spot
        float angle = std::uniform_real_distribution<float>(0, 2 * M_PI)(rng);
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
            if (isValidSpawnPosition(spawnPos, gameManager)) {
                return spawnPos;
            }
        }
    }
    // Fallback: if no good off-screen spot found, spawn near player but try to be outside immediate view
    // This part of fallback might need refinement or could just spawn at playerPos + offset if all else fails
    float fallbackAngle = std::uniform_real_distribution<float>(0, 2 * M_PI)(rng);
    return playerPos + sf::Vector2f(std::cos(fallbackAngle) * (viewRect.width / 2 + minSpawnDistFromScreenEdge),
        std::sin(fallbackAngle) * (viewRect.height / 2 + minSpawnDistFromScreenEdge));
}


void PoliceManager::managePolicePopulation(int wantedLevel, const sf::Vector2f& playerPos, const sf::View& gameView, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons)
{
    m_desiredBatonOfficers = 0;
    m_desiredPistolOfficers = 0;
    m_desiredPoliceCars = 0;
    m_desiredPoliceHelicopters = 0;
    m_desiredPoliceTanks = 0;

    // Determine desired number of DYNAMIC units based on wanted level
    switch (wantedLevel) {
    case 0: // No dynamic units at 0 stars
        m_desiredBatonOfficers = 0;
        m_desiredPistolOfficers = 0;
        m_desiredPoliceCars = 0;
        m_desiredPoliceHelicopters = 0;
        m_desiredPoliceTanks = 0;
        break;
    case 1:
        m_desiredBatonOfficers = 2;
        m_desiredPistolOfficers = 0;
        m_desiredPoliceCars = 0;
        break;
    case 2:
        m_desiredBatonOfficers = 3;
        m_desiredPistolOfficers = 2;
        m_desiredPoliceCars = 1;
        break;
    case 3:
        m_desiredBatonOfficers = 2;
        m_desiredPistolOfficers = 3;
        m_desiredPoliceCars = 2;
        m_desiredPoliceHelicopters = 0; // Helicopters start at 4 stars in this setup
        break;
    case 4:
        m_desiredPistolOfficers = 4; // More pistol officers
        m_desiredPoliceCars = 3;
        m_desiredPoliceHelicopters = 1;
        m_desiredBatonOfficers = 1; // Fewer baton officers at higher levels
        break;
    case 5:
        m_desiredPistolOfficers = 5;//5
        m_desiredPoliceCars = 3;//3
        m_desiredPoliceHelicopters = 2;//2
        m_desiredPoliceTanks = 1;//change to 1...
        m_desiredBatonOfficers = 0;
        break;
    default: // For wanted levels > 5
        m_desiredPistolOfficers = 6;
        m_desiredPoliceCars = 4;
        m_desiredPoliceHelicopters = 2;
        m_desiredPoliceTanks = 2;
        m_desiredBatonOfficers = 0;
        break;
    }

    const float minSpawnDistFromPlayer = std::max(gameView.getSize().x, gameView.getSize().y) * 0.6f;
    const float minSpawnDistFromScreenEdge = 50.f;

    // --- Spawning Logic (Dynamic Units Only) ---
    // Count only DYNAMIC officers for these checks
    int currentDynamicBatonOfficers = 0;
    int currentDynamicPistolOfficers = 0;
    for (const auto& officer : m_policeOfficers) {
        if (!officer->isStatic()) {
            if (officer->getWeaponType() == PoliceWeaponType::BATON) currentDynamicBatonOfficers++;
            else if (officer->getWeaponType() == PoliceWeaponType::PISTOL) currentDynamicPistolOfficers++;
        }
    }

    int currentDynamicPoliceCars = 0;
    for (const auto& car : m_policeCars) {
        if (!car->isStatic()) {
            currentDynamicPoliceCars++;
        }
    }
    // Helicopter and Tank counts are assumed to be only dynamic already.

    // Spawn Baton Officers if needed
    if (countDynamicPoliceByType(PoliceWeaponType::BATON) < m_desiredBatonOfficers && m_batonOfficerSpawnTimer <= 0.f) {
        sf::Vector2f spawnPos = findOffScreenSpawnPosition(gameView, playerPos, minSpawnDistFromPlayer, minSpawnDistFromScreenEdge, blockedPolygons, m_gameManager);
        spawnPolice(spawnPos, PoliceWeaponType::BATON, false);
        m_batonOfficerSpawnTimer = SPAWN_COOLDOWN_SECONDS;
    }

    // Spawn Pistol Officers if needed
    if (countDynamicPoliceByType(PoliceWeaponType::PISTOL) < m_desiredPistolOfficers && m_pistolOfficerSpawnTimer <= 0.f) {
        sf::Vector2f spawnPos = findOffScreenSpawnPosition(gameView, playerPos, minSpawnDistFromPlayer, minSpawnDistFromScreenEdge, blockedPolygons, m_gameManager);
        spawnPolice(spawnPos, PoliceWeaponType::PISTOL, false);
        m_pistolOfficerSpawnTimer = SPAWN_COOLDOWN_SECONDS;
    }

    // Spawn Police Cars if needed
    if (countDynamicPoliceCars() < m_desiredPoliceCars && m_policeCarSpawnTimer <= 0.f) {
        sf::Vector2f spawnPos = findOffScreenSpawnPosition(gameView, playerPos, minSpawnDistFromPlayer, minSpawnDistFromScreenEdge, blockedPolygons, m_gameManager);
        spawnPoliceCar(spawnPos, false);
        m_policeCarSpawnTimer = SPAWN_COOLDOWN_SECONDS;
    }

    // Spawn Helicopters if needed
    if (countPoliceHelicopters() < m_desiredPoliceHelicopters && m_policeHelicopterSpawnTimer <= 0.f) {
        sf::Vector2f spawnPos = findOffScreenSpawnPosition(gameView, playerPos, minSpawnDistFromPlayer + 100.f, minSpawnDistFromScreenEdge + 50.f, {}, m_gameManager);
        spawnPoliceHelicopter(spawnPos);
        m_policeHelicopterSpawnTimer = SPAWN_COOLDOWN_SECONDS * 2;
    }

    // Spawn Tanks if needed
    if (countPoliceTanks() < m_desiredPoliceTanks && m_policeTankSpawnTimer <= 0.f) {
        sf::Vector2f spawnPos = findOffScreenSpawnPosition(gameView, playerPos, minSpawnDistFromPlayer + 50.f, minSpawnDistFromScreenEdge, blockedPolygons, m_gameManager);
        spawnPoliceTank(spawnPos);
        m_policeTankSpawnTimer = SPAWN_COOLDOWN_SECONDS * 3;
    }

    // --- Despawning Logic (Retreat for Dynamic Units Only) ---
    int batonOfficersToRetreat = std::max(0, countDynamicPoliceByType(PoliceWeaponType::BATON) - m_desiredBatonOfficers);
    int pistolOfficersToRetreat = std::max(0, countDynamicPoliceByType(PoliceWeaponType::PISTOL) - m_desiredPistolOfficers);
    int policeCarsToRetreat = std::max(0, countDynamicPoliceCars() - m_desiredPoliceCars);
    // Similar logic for helicopters and tanks if they need to retreat instead of just vanishing.
    int retreated = 0;
    // Make Baton Officers retreat
    for (auto& officer : m_policeOfficers) {
        if (batonOfficersToRetreat <= 0 || retreated >= MAX_RETREATS_PER_FRAME) break;
        if (!officer->isStatic() && officer->getWeaponType() == PoliceWeaponType::BATON && !officer->isRetreating()) {
            officer->needsCleanup = true; // Remove instantly to avoid heavy pathfinding
            batonOfficersToRetreat--;
            retreated++;
        }
    }
    retreated = 0;
    // Make Pistol Officers retreat
    for (auto& officer : m_policeOfficers) {
        if (pistolOfficersToRetreat <= 0 || retreated >= MAX_RETREATS_PER_FRAME) break;
        if (!officer->isStatic() && officer->getWeaponType() == PoliceWeaponType::PISTOL && !officer->isRetreating()) {
            officer->needsCleanup = true;
            pistolOfficersToRetreat--;
            retreated++;
        }
    }
    retreated = 0;
    // Make Police Cars retreat
    for (auto& car : m_policeCars) {
        if (policeCarsToRetreat <= 0 || retreated >= MAX_RETREATS_PER_FRAME) break;
        if (!car->isStatic() && !car->isRetreating() && !car->isAmbient()) {
            sf::Vector2f retreatPos = findOffScreenSpawnPosition(gameView, playerPos, minSpawnDistFromPlayer, minSpawnDistFromScreenEdge, blockedPolygons, m_gameManager);
            car->startRetreating(retreatPos);
            policeCarsToRetreat--;
            retreated++;
        }
    }
    // Note: Helicopters and Tanks currently don't have a retreat mechanic implemented beyond despawning when out of sight or destroyed.
    // If they also need to "retreat", similar logic would be added for them.
}


void PoliceManager::retreatAllCars(Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    const sf::View& view = m_gameManager.getGameView();
    sf::Vector2f playerPos = player.getPosition();

    auto issueRetreat = [&](std::unique_ptr<PoliceCar>& car) {
        if (car->getDriver() == &player || car->isRetreating())
            return;
        sf::Vector2f target = findOffScreenSpawnPosition(view, playerPos, 300.f, 150.f, blockedPolygons, m_gameManager);
        car->startRetreating(target);
        };

    for (auto& car : m_policeCars) {
        issueRetreat(car);
    }
    for (auto& car : m_staticPoliceCars) {
        issueRetreat(car);
    }
}

// Spawn functions for new units
void PoliceManager::spawnPoliceHelicopter(const sf::Vector2f& position) {
    m_policeHelicopters.push_back(std::make_unique<PoliceHelicopter>(m_gameManager, position));
    // std::cout << "Spawned Police Helicopter at: (" << position.x << ", " << position.y << ")\n";
}

void PoliceManager::spawnPoliceTank(const sf::Vector2f& position) {
    sf::Vector2f spawnPos = position; // Fallback position
    const RoadSegment* chosenRoad = nullptr;
    int chosenLaneIndex = 0;
    float roadAngle = 0.f; // Default angle

    CarManager* cm = m_gameManager.getCarManager();
    if (cm && !cm->getRoads().empty()) {
        const auto& roads = cm->getRoads();
        int roadIdx = rand() % roads.size();
        chosenRoad = &roads[roadIdx];
        chosenLaneIndex = rand() % std::max(1, chosenRoad->lanes);
        spawnPos = chosenRoad->getLaneCenter(chosenLaneIndex);

        // Determine angle from road direction
        // Assuming tank sprite's front is oriented "upwards" (points to top of texture)
        // and PoliceTank::updateMovement adds 90 degrees to its calculated angle.
        // So, we need to subtract 90 from the world angle.
        if (chosenRoad->direction == "up") { // World angle -90
            roadAngle = -180.f; // Sprite rotation -90 - 90 = -180
        }
        else if (chosenRoad->direction == "down") { // World angle 90
            roadAngle = 0.f;    // Sprite rotation 90 - 90 = 0
        }
        else if (chosenRoad->direction == "left") { // World angle 180
            roadAngle = 90.f;   // Sprite rotation 180 - 90 = 90
        }
        else if (chosenRoad->direction == "right") { // World angle 0
            roadAngle = -90.f;  // Sprite rotation 0 - 90 = -90
        }
        // std::cout << "Spawning tank on road: " << chosenRoad->direction << " world_angle_equivalent: " << roadAngle + 90.f << " sprite_angle: " << roadAngle << std::endl;

    }
    else {
        std::cerr << "PoliceManager: Could not find roads to spawn tank. Using fallback position." << std::endl;
        // Fallback orientation: assuming "up" if tank sprite is up, so -90 to make it face right by default, or 0 if movement code handles it.
        // Given the +90 in movement, if we want it to face "right" by default without a road, set to -90.
        // If we want it to face "up" (sprite's natural orientation), set to 0.
        // Let's assume facing "up" (sprite's default) is acceptable if no road.
        roadAngle = 0.f;
    }

    auto newTank = std::make_unique<PoliceTank>(m_gameManager, spawnPos);
    newTank->getSprite().setRotation(roadAngle); // Set initial rotation of the tank body
    // Turret usually aims at player, so its initial rotation might be less critical here or set by its own logic.
    // If the turret should also align with the road initially:
    // newTank->getTurretSprite().setRotation(roadAngle); 

    m_policeTanks.push_back(std::move(newTank));
    // std::cout << "Spawned Police Tank at: (" << spawnPos.x << ", " << spawnPos.y << ") angle: " << roadAngle << "\n";
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

const std::vector<std::unique_ptr<PoliceTank>>& PoliceManager::getPoliceTanks() const {
    return m_policeTanks;
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