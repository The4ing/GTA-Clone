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

class PoliceManager {
public:
    PoliceManager(GameManager& gameManager);

    void spawnPolice(const sf::Vector2f& position, PoliceWeaponType weaponType);
    void spawnPoliceCar(const sf::Vector2f& position);
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

    void managePolicePopulation(int wantedLevel, const sf::Vector2f& playerPos);
    int countPoliceByType(PoliceWeaponType type);
    int countPoliceCars();
    int countPoliceHelicopters();
    int countPoliceTanks();

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

    const float SPAWN_COOLDOWN_SECONDS = 2.0f;

    std::vector<sf::Vector2i> activeChunks;
    float spawnCooldown = 0.f;
};
