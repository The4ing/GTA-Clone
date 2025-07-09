#pragma once

#include "Vehicle.h"
#include "Pathfinder.h"
#include <SFML/Graphics.hpp>
#include <vector>

class GameManager;
class Player;

class PoliceTank : public Vehicle {
public:
    PoliceTank(GameManager& gameManager, const sf::Vector2f& startPosition);

    void update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void draw(sf::RenderTarget& target) override;

    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;

    bool isDestroyed() const;
    void takeDamage(int amount);
    bool attemptRunOverPedestrian(class Pedestrian& ped);
    bool attemptRunOverVehicle(class Vehicle& vehicle);
    bool isReadyForCleanup() const { return m_readyForCleanup; }

private:
    void updateAIBehavior(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void updateMovement(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void aimAndFire(Player& player, float dt);
    bool hasClearLineOfSight(const sf::Vector2f& targetPos, const QuadTree<std::vector<sf::Vector2f>>& blockedPolyTree) const;

    GameManager& m_gameManager;
    Pathfinder m_pathfinder;
    sf::Sprite m_turretSprite;
    bool m_hasLineOfSightToPlayer = false; // Added to store LOS state
    float m_distanceToPlayer = 0.f; // Distance from tank to player each frame

    const float STOP_DISTANCE = 300.f; // distance at which the tank stops moving

    int m_health = 500;
    float m_speed = 10.f;
    float m_rotationSpeed = 30.f;
    float m_turretRotationSpeed = 15.f;

    std::vector<sf::Vector2f> m_currentPath;
    size_t m_currentPathIndex = 0;
    float m_repathTimer = 0.f;
    sf::Vector2f m_targetPosition; // Current target (player or retreat point)

    float m_cannonCooldownTimer = 0.f;
    const float CANNON_FIRE_RATE = 5.0f;
    const float CANNON_RANGE = 400.f;

    // Despawning logic members
    bool m_isRetreatingToDespawn = false;
    bool m_readyForCleanup = false;
    // sf::Vector2f m_retreatTargetPosition; // Not strictly needed if m_targetPosition is reused
};
