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

    void takeDamage(int amount);

    bool attemptRunOverPedestrian(Pedestrian& ped); // Changed parameter to non-const class
    bool attemptRunOverVehicle(Vehicle& vehicle); // Changed parameter to non-const class
    bool isDestroyed() const;
    bool isReadyForCleanup() const;
    bool canSeePlayer(const Player& player, const std::vector<std::vector<sf::Vector2f>>& obstacles);

    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;
   

private:
    // Copied from PoliceCar.h
    enum class TankState { Chasing, Retreating }; // Simplified states for Tank
    enum class MovementAxis { None, Horizontal, Vertical };

    TankState m_tankState = TankState::Chasing;
    MovementAxis m_previousMovementAxis = MovementAxis::None;

    void updateAIBehavior(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void updateTankMovementAsCar(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons); // New movement function
    void aimAndFire(Player& player, float dt);

    bool hasClearLineOfSight(const sf::Vector2f& targetPos, const QuadTree<std::vector<sf::Vector2f>>& blockedPolyTree) const;
    bool handlesOwnAIRotation() const override;

  
    GameManager& m_gameManager;
    Pathfinder m_pathfinder;
    sf::Sprite m_turretSprite;

    std::vector<sf::Vector2f> getVisibleHitboxCorners() const;
    std::vector<sf::Vector2f> m_currentPath;
    sf::Vector2f m_currentTargetPosition;
    sf::Vector2f m_bezierP0, m_bezierP1, m_bezierP2;

    size_t m_currentPathIndex;

    bool m_hasLineOfSightToPlayer ;
    bool m_usingBezier ;
    bool m_readyForCleanup ;
    bool m_wasUsingBezierLastFrame ;
   
    float m_distanceToPlayer;
    int m_health ;
    float m_currentSpeed ;
    float m_turretRotationSpeed;
    float m_repathTimer;
    float m_cannonCooldownTimer ;
    float m_visionDistance ;
    float m_fieldOfViewAngle ; // Degrees
    float m_bezierT ;
    float m_baseSpeed ;


    
};
