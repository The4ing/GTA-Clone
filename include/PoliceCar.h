#pragma once
#include "Vehicle.h"
#include "Pathfinder.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include "Constants.h"

class GameManager;
class Player;
class PatrolZone; // Forward declaration

class PoliceCar : public Vehicle {
public:
    PoliceCar(GameManager& gameManager, const sf::Vector2f& startPosition);

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

private:
    void updateChaseBehavior(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    bool attemptRunOverPlayer(Player& player);

    GameManager& m_gameManager;
    bool m_isAmbient = true;

    Pathfinder m_pathfinder;
    sf::Sprite m_sprite;

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
