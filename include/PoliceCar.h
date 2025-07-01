#pragma once

#include "Vehicle.h" // Inherit from Vehicle for more robust movement and player interaction
#include "Pathfinder.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include "Constants.h"

class GameManager; // Forward declaration for accessing game systems (e.g., pathfinding grid)
class Player;      // Forward declaration for targeting the player

class PoliceCar : public Vehicle { // Changed inheritance from MovingObject to Vehicle
public:
    PoliceCar(GameManager& gameManager, const sf::Vector2f& startPosition);

    // Update method now takes the player and blocked polygons for pathfinding and collision
    void update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);

    // draw method signature is usually (sf::RenderTarget& target) in SFML projects, not RenderWindow
    void draw(sf::RenderTarget& target) override;

    // setTarget might be implicitly handled by update chasing the player
    // void setTarget(const sf::Vector2f& target); // Keep if direct targeting is needed

    // Overrides from Vehicle or GameObject if necessary
    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;
    // Add other necessary overrides if Vehicle defines them as pure virtual

private:
    void updateChaseBehavior(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    bool attemptRunOverPlayer(Player& player); // Logic for trying to hit the player

    GameManager& m_gameManager; // Reference to GameManager
    Pathfinder m_pathfinder;    // Pathfinder for navigating
    sf::Sprite m_sprite;        // Sprite for the police car (Vehicle class might also have one)
    // If Vehicle handles its own sprite, this might be redundant or for specific parts.
    // For now, assuming PoliceCar manages its visual representation.

    std::vector<sf::Vector2f> m_currentPath; // Path to the target
    size_t m_currentPathIndex = 0;           // Current waypoint in the path
    float m_repathTimer = 0.f;               // Timer to manage path recalculation frequency
    sf::Vector2f m_currentTargetPosition;    // The position the car is currently pathfinding towards

    float m_speed = 120.f; // Speed of the police car, potentially override Vehicle's speed
    // float m_turnSpeed = 100.f; // Example: degrees per second, if using Vehicle's steering
    float m_bumpCooldown = 0.f;

    // Collision properties (if not fully handled by Vehicle)
    // float m_collisionRadius; 

    // Constants for AI behavior
    const float REPATH_COOLDOWN = 1.0f; // Seconds before trying to find a new path
    const float PLAYER_MOVE_THRESHOLD_FOR_REPATH_SQ = (PATHFINDING_GRID_SIZE * 2.0f) * (PATHFINDING_GRID_SIZE * 2.0f);
    const float TARGET_REACHED_DISTANCE = PATHFINDING_GRID_SIZE; // How close to get to a waypoint
    const float RUN_OVER_DISTANCE = 30.f; // Distance to consider a successful run-over attempt
};
