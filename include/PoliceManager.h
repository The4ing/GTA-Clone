
/*
#pragma once
#include "Police.h"
#include "PoliceCar.h"
#include "PoliceHelicopter.h"
#include "PoliceTank.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <SFML/System/Vector2.hpp>
#include "Constants.h"
#include "QuadTree.h"

class GameManager;
class Player;
class PatrolZone; // Forward declarationAdd commentMore actions
struct RoadSegment; // Forward declaration

class PoliceManager {
public:
    PoliceManager(GameManager& gameManager);

    Police* spawnPatrolOfficer(const sf::Vector2f& position, PoliceWeaponType weaponType, PatrolZone* zone); // For static patrol
    PoliceCar* spawnPatrolCar(const sf::Vector2f& position, PatrolZone* zone); // For static patrol car

    void spawnPolice(const sf::Vector2f& position, PoliceWeaponType weaponType); // General dynamic spawn (aggressive)
    void spawnPoliceCar(const sf::Vector2f& position); // Spawns an aggressive police car (dynamic)
    void spawnAmbientPoliceCarOnRoadSegment(const RoadSegment* road, int laneIndex, const std::string& actualDir, const sf::Vector2f& spawnPosition);
    void spawnPoliceHelicopter(const sf::Vector2f& position);
    void spawnPoliceTank(const sf::Vector2f& position);

    void update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);

    void draw(sf::RenderTarget& window);
    void damageClosestOfficer(const sf::Vector2f& pos, int amount);

    void trySpawnRandomPoliceNear(const std::vector<sf::Vector2i>& activeChunks, const sf::Vector2f& playerPos);

    static bool canRequestPath();
    static void recordPathfindingCall();
    static void resetPathfindingCounter();

    void clearAllPolice();
    const std::vector<std::unique_ptr<PoliceCar>>& getPoliceCars() const;

private:
    void updatePoliceOfficers(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void updatePoliceCars(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void updatePoliceHelicopters(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void updatePoliceTanks(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);

    void managePolicePopulation(int wantedLevel, const sf::Vector2f& playerPos, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);


    int countPoliceByType(PoliceWeaponType type);
    int countPoliceCars();
    int countPoliceHelicopters();
    int countPoliceTanks();
    bool isInsideBlockedPolygon(const sf::Vector2f& position, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);


    GameManager& m_gameManager;

    std::vector<std::unique_ptr<Police>> m_policeOfficers;
    std::vector<std::unique_ptr<PoliceCar>> m_policeCars;
    std::vector<std::unique_ptr<PoliceHelicopter>> m_policeHelicopters;
    std::vector<std::unique_ptr<PoliceTank>> m_policeTanks;

    float m_batonOfficerSpawnTimer = 0.f;
    float m_pistolOfficerSpawnTimer = 0.f;
    float m_policeCarSpawnTimer = 0.f;
    float m_policeHelicopterSpawnTimer = 0.f;
    float m_policeTankSpawnTimer = 0.f;

    int m_desiredBatonOfficers = 0;
    int m_desiredPistolOfficers = 0;
    int m_desiredPoliceCars = 0;
    int m_desiredPoliceHelicopters = 0;
    int m_desiredPoliceTanks = 0;


    int m_numSeeingPlayer = 0; 
    float m_timePlayerNotSeen = 0.0f;
    float m_wantedReductionCooldownTimer = 0.0f; // Cooldown between successive wanted level reductions

    std::vector<sf::Vector2i> activeChunks;
    float spawnCooldown = 0.f;
};

*/

#pragma once
#include "Police.h"
#include "PoliceCar.h"
#include "PoliceHelicopter.h"
#include "PoliceTank.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <SFML/System/Vector2.hpp>
#include "Constants.h"
#include "QuadTree.h"

class GameManager;
class Player;
class PatrolZone; // Forward declarationAdd commentMore actions
struct RoadSegment; // Forward declaration

class PoliceManager {
public:
    PoliceManager(GameManager& gameManager);

    Police* spawnPatrolOfficer(const sf::Vector2f& position, PoliceWeaponType weaponType, PatrolZone* zone); // For static patrol
    PoliceCar* spawnPatrolCar(const sf::Vector2f& position, PatrolZone* zone); // For static patrol car

    void spawnStaticPoliceUnits(const sf::FloatRect& mapBounds, float gridSize, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);

    void spawnPolice(const sf::Vector2f& position, PoliceWeaponType weaponType, bool isStatic = false); // General dynamic spawn (aggressive)
    void spawnPoliceCar(const sf::Vector2f& position, bool isStatic = false); // Spawns an aggressive police car (dynamic)
    void spawnAmbientPoliceCarOnRoadSegment(const RoadSegment* road, int laneIndex, const std::string& actualDir, const sf::Vector2f& spawnPosition);
    void spawnPoliceHelicopter(const sf::Vector2f& position);
    void spawnPoliceTank(const sf::Vector2f& position);

    void update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);

    void draw(sf::RenderTarget& window);
    void damageClosestOfficer(const sf::Vector2f& pos, int amount);

    void trySpawnRandomPoliceNear(const std::vector<sf::Vector2i>& activeChunks, const sf::Vector2f& playerPos);

    static bool canRequestPath();
    static void recordPathfindingCall();
    static void resetPathfindingCounter();

    void clearAllPolice();
    const std::vector<std::unique_ptr<PoliceCar>>& getPoliceCars() const;

private:
    void updatePoliceOfficers(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void updatePoliceCars(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void updatePoliceHelicopters(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void updatePoliceTanks(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);

    void managePolicePopulation(int wantedLevel, const sf::Vector2f& playerPos, const sf::View& gameView, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);


    int countPoliceByType(PoliceWeaponType type); // Counts all (static + dynamic)
    int countPoliceCars(); // Counts all (static + dynamic)
    int countDynamicPoliceByType(PoliceWeaponType type); // Counts only dynamic
    int countDynamicPoliceCars(); // Counts only dynamic
    int countPoliceHelicopters(); // Assumed dynamic only
    int countPoliceTanks();       // Assumed dynamic only
    bool isInsideBlockedPolygon(const sf::Vector2f& position, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);


    GameManager& m_gameManager;

    std::vector<std::unique_ptr<Police>> m_policeOfficers; // Dynamic officers
    std::vector<std::unique_ptr<PoliceCar>> m_policeCars; // Dynamic cars
    std::vector<std::unique_ptr<PoliceHelicopter>> m_policeHelicopters;
    std::vector<std::unique_ptr<PoliceTank>> m_policeTanks;

    std::vector<std::unique_ptr<Police>> m_staticPoliceOfficers; // Static officers
    std::vector<std::unique_ptr<PoliceCar>> m_staticPoliceCars;   // Static cars

    float m_batonOfficerSpawnTimer = 0.f;
    float m_pistolOfficerSpawnTimer = 0.f;
    float m_policeCarSpawnTimer = 0.f;
    float m_policeHelicopterSpawnTimer = 0.f;
    float m_policeTankSpawnTimer = 0.f;

    int m_desiredBatonOfficers = 0;
    int m_desiredPistolOfficers = 0;
    int m_desiredPoliceCars = 0;
    int m_desiredPoliceHelicopters = 0;
    int m_desiredPoliceTanks = 0;


    int m_numSeeingPlayer = 0;
    float m_timePlayerNotSeen = 0.0f;
    float m_wantedReductionCooldownTimer = 0.0f; // Cooldown between successive wanted level reductions

    std::vector<sf::Vector2i> activeChunks;
    float spawnCooldown = 0.f;
};

