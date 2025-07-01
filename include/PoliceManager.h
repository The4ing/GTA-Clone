#pragma once
#include "Police.h"
#include "PoliceCar.h"
#include "PoliceHelicopter.h" // Added for PoliceHelicopter
#include "PoliceTank.h"       // Added for PoliceTank
#include <vector>
#include <memory>
#include <unordered_map>
#include <SFML/System/Vector2.hpp>
#include "Constants.h"
#include "QuadTree.h" // Assuming QuadTree might be used for police units, if not, this is just existing.

class GameManager;
class Player; // Forward declaration

class PoliceManager {
public:
    PoliceManager(GameManager& gameManager);

    // Modified spawnPolice to take weapon type
    void spawnPolice(const sf::Vector2f& position, PoliceWeaponType weaponType);
    void spawnPoliceCar(const sf::Vector2f& position);
    void spawnPoliceHelicopter(const sf::Vector2f& position); // Added for spawning helicopters
    void spawnPoliceTank(const sf::Vector2f& position);       // Added for spawning tanks

    void update(float dt, Player& player, // Changed to pass Player reference
        const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);

    void draw(sf::RenderTarget& window);
    void damageClosestOfficer(const sf::Vector2f& pos, int amount); // Renamed for clarity

    // This might be deprecated or heavily modified by wanted level logic
    void trySpawnRandomPoliceNear(const std::vector<sf::Vector2i>& activeChunks, const sf::Vector2f& playerPos);

    static bool canRequestPath();
    static void recordPathfindingCall();
    static void resetPathfindingCounter();

    void clearAllPolice(); // Utility to remove all police units

private:
    void updatePoliceOfficers(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons); // Renamed
    void updatePoliceCars(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void updatePoliceHelicopters(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons); // Added
    void updatePoliceTanks(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);       // Added

    void managePolicePopulation(int wantedLevel, const sf::Vector2f& playerPos);
    int countPoliceByType(PoliceWeaponType type);
    int countPoliceCars();
    int countPoliceHelicopters(); // Added
    int countPoliceTanks();       // Added

    GameManager& m_gameManager;

    std::vector<std::unique_ptr<Police>> m_policeOfficers;
    std::vector<std::unique_ptr<PoliceCar>> m_policeCars;
    std::vector<std::unique_ptr<PoliceHelicopter>> m_policeHelicopters;
    std::vector<std::unique_ptr<PoliceTank>> m_policeTanks;

    // Timers or counters for managing spawn rates per wanted level
    float m_batonOfficerSpawnTimer = 0.f;
    float m_pistolOfficerSpawnTimer = 0.f;
    float m_policeCarSpawnTimer = 0.f;
    float m_policeHelicopterSpawnTimer = 0.f; // Added
    float m_policeTankSpawnTimer = 0.f;       // Added

    // Desired counts for current wanted level (updated each frame based on wanted level)
    int m_desiredBatonOfficers = 0;
    int m_desiredPistolOfficers = 0;
    int m_desiredPoliceCars = 0;
    int m_desiredPoliceHelicopters = 0; // Added
    int m_desiredPoliceTanks = 0;       // Added

    // Cooldowns for spawning types of units
    const float SPAWN_COOLDOWN_SECONDS = 2.0f; // General cooldown between spawns of a type

    // Max units per type (can be part of wanted level logic)
    // const int MAX_BATON_OFFICERS = 10;
    // const int MAX_PISTOL_OFFICERS = 10;
    // const int MAX_POLICE_CARS = 10;


    // This existing member seems to be for random ambient spawning, might need adjustment
    std::vector<sf::Vector2i> activeChunks;
    float spawnCooldown = 0.f; // This might be the old random spawn timer
};
