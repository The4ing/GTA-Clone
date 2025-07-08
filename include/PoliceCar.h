#pragma once
#include "Vehicle.h"
#include "Pathfinder.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include "Constants.h"

class GameManager;
class Player;
class PatrolZone; // Forward declaration
class Pedestrian;

class PoliceCar : public Vehicle {
public:
    PoliceCar(GameManager& gameManager, const sf::Vector2f& startPosition);
    ~PoliceCar();

    void setPatrolZone(PatrolZone* zone);
    PatrolZone* getPatrolZone() const;

    bool canSeePlayer(const Player& player, const std::vector<std::vector<sf::Vector2f>>& obstacles);

    void update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);

    void draw(sf::RenderTarget& target) override;

    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;

    void setIsAmbient(bool isAmbient);
    bool isAmbient() const;
    bool m_playerCausedWantedIncrease = false;
    bool hasOfficerInside() const { return m_hasOfficerInside; }
    void setOfficerInside(bool inside) { m_hasOfficerInside = inside; }
    bool readyForOfficerExit() const;
    void clearOfficerExitRequest();
    void startRetreating(const sf::Vector2f& retreatTarget);
    bool isRetreating() const; // Implementation will check internal state
    bool needsCleanup = false;
    void setIsStatic(bool isStatic) { m_isStatic = isStatic; }
    bool isStatic() const { return m_isStatic; }
    bool attemptRunOverPedestrian(Pedestrian& ped);


private:
    enum class CarState { Chasing, AmbientDriving, Retreating };
    CarState m_carState = CarState::AmbientDriving;
    void updateChaseBehavior(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    bool attemptRunOverPlayer(Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);

    bool m_isStatic = false; // Flag to mark static police cars
    int m_playerHitCount = 0; // Tracks hits specifically against the player
    bool m_hasOfficerInside = true;
    GameManager& m_gameManager;
    bool m_isAmbient = true;
    sf::Sound m_sirenSound;

    Pathfinder m_pathfinder;
    sf::Sprite m_sprite;
    float m_currentSpeed = 120.f;
    int m_bumpCount = 0;
    bool m_requestOfficerExit = false;

    std::vector<sf::Vector2f> m_currentPath;
    size_t m_currentPathIndex = 0;
    float m_repathTimer = 0.f;
    sf::Vector2f m_currentTargetPosition;

    float m_speed = 120.f;
    float m_bumpCooldown = 0.f;

    const float REPATH_COOLDOWN = 1.0f;
    const float PLAYER_MOVE_THRESHOLD_FOR_REPATH_SQ = (PATHFINDING_GRID_SIZE * 2.0f) * (PATHFINDING_GRID_SIZE * 2.0f);
    const float TARGET_REACHED_DISTANCE = PATHFINDING_GRID_SIZE;
    const float RUN_OVER_DISTANCE = 30.f;
    PatrolZone* m_assignedZone = nullptr;

    // Vision parameters (can be different from foot police)
    float m_visionDistance = 250.f;
    float m_fieldOfViewAngle = 140.f; // Degrees
};
