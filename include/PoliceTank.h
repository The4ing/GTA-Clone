#pragma once

#include "Vehicle.h"
#include "Pathfinder.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include "Constants.h" // Added for PATHFINDING_GRID_SIZE

class GameManager;
class Player;
class Pedestrian; // Forward declaration

class PoliceTank : public Vehicle {
public:
    PoliceTank(GameManager& gameManager, const sf::Vector2f& startPosition);

    void update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void draw(sf::RenderTarget& target) override;

    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;

    bool isDestroyed() const;
    void takeDamage(int amount);
    bool attemptRunOverPedestrian(Pedestrian& ped); // Changed parameter to non-const class
    bool attemptRunOverVehicle(Vehicle& vehicle); // Changed parameter to non-const class
    bool isReadyForCleanup() const { return m_readyForCleanup; }
    bool canSeePlayer(const Player& player, const std::vector<std::vector<sf::Vector2f>>& obstacles);

private:
    // Copied from PoliceCar.h
    enum class TankState { Chasing, Retreating }; // Simplified states for Tank
    TankState m_tankState = TankState::Chasing;

    void updateAIBehavior(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    // void updateMovement(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons); // Will be removed
    void updateTankMovementAsCar(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons); // New movement function
    void aimAndFire(Player& player, float dt);
    bool hasClearLineOfSight(const sf::Vector2f& targetPos, const QuadTree<std::vector<sf::Vector2f>>& blockedPolyTree) const;

    GameManager& m_gameManager;
    Pathfinder m_pathfinder;
    sf::Sprite m_turretSprite;
    bool m_hasLineOfSightToPlayer = false;
    float m_distanceToPlayer = 0.f;

    const float STOP_DISTANCE = 300.f; // distance at which the tank stops moving if it has LOS.

    int m_health = 500;
    // float m_speed = 10.f; // Base speed, potentially overridden or used by new logic. Will use Vehicle's speed or define similar to PoliceCar
    float m_currentSpeed = 10.f; // Current operational speed, similar to PoliceCar's m_currentSpeed
    // float m_rotationSpeed = 30.f; // Will be handled by car-like movement logic.
    float m_turretRotationSpeed = 15.f;

    std::vector<sf::Vector2f> m_currentPath;
    size_t m_currentPathIndex = 0;
    float m_repathTimer = 0.f;
    // sf::Vector2f m_targetPosition; // Current target for pathfinding (player or retreat point) - Renamed to m_currentTargetPosition
    sf::Vector2f m_currentTargetPosition; // Current target for pathfinding, similar to PoliceCar

    float m_cannonCooldownTimer = 0.f;
    const float CANNON_FIRE_RATE = 5.0f;
    const float CANNON_RANGE = 400.f;
    float m_visionDistance = 300.f;
    float m_fieldOfViewAngle = 160.f; // Degrees
    bool m_usingBezier = false;
    float m_bezierT = 0.f;
    sf::Vector2f m_bezierP0, m_bezierP1, m_bezierP2;


    // Despawning logic members (integrates with m_tankState)
    // bool m_isRetreatingToDespawn = false; // This concept will be handled by m_tankState == TankState::Retreating
    bool m_readyForCleanup = false;

    // Constants adapted from PoliceCar for movement behavior
    const float REPATH_COOLDOWN = 1.0f; // Matches PoliceCar
    const float PLAYER_MOVE_THRESHOLD_FOR_REPATH_SQ = (PATHFINDING_GRID_SIZE * 2.0f) * (PATHFINDING_GRID_SIZE * 2.0f); // Matches PoliceCar
    const float TARGET_REACHED_DISTANCE = PATHFINDING_GRID_SIZE; // Matches PoliceCar

    // Base speed for the tank, analogous to m_speed in PoliceCar
    float m_baseSpeed = 10.f; // Renamed from m_speed to avoid confusion if Vehicle::speed is used differently

    enum class MovementAxis { None, Horizontal, Vertical };
    MovementAxis m_previousMovementAxis = MovementAxis::None;
    bool m_wasUsingBezierLastFrame = false; // For logging Bezier starts

    bool handlesOwnAIRotation() const override { return true; }
};
