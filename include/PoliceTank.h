#pragma once

#include "Vehicle.h" // Tanks are vehicles
#include "Pathfinder.h" // For navigation
#include <SFML/Graphics.hpp>
#include <vector>

class GameManager;
class Player;

class PoliceTank : public Vehicle {
public:
    PoliceTank(GameManager& gameManager, const sf::Vector2f& startPosition);

    void update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) ;
    void draw(sf::RenderTarget& target) override;

    // Overrides from Vehicle or GameObject
    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;

    bool isDestroyed() const;
    void takeDamage(int amount);

private:
    void updateAIBehavior(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void updateMovement(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void aimAndFire(Player& player, float dt);

    GameManager& m_gameManager;
    Pathfinder m_pathfinder; // Tanks will need to navigate
    sf::Sprite m_turretSprite; // Separate sprite for turret rotation

    // Tank specific properties
    int m_health = 500; // Tanks are very tough
    float m_speed = 30.f;  // Slower than cars
    float m_rotationSpeed = 45.f; // Degrees per second for tank body
    float m_turretRotationSpeed = 60.f; // Degrees per second for turret

    // Pathfinding and movement
    std::vector<sf::Vector2f> m_currentPath;
    size_t m_currentPathIndex = 0;
    float m_repathTimer = 0.f;
    sf::Vector2f m_targetPosition; // Current pathfinding target (player position)

    // Weapon properties
    float m_cannonCooldownTimer = 0.f;
    const float CANNON_FIRE_RATE = 5.0f; // Seconds between cannon shots
    const float CANNON_RANGE = 400.f;
    // const int CANNON_DAMAGE = 100; // Damage would be handled by projectile
    // const float CANNON_PROJECTILE_SPEED = 300.f;

    // If Vehicle class does not have its own sprite, Tank will need one for its body
    // sf::Sprite m_bodySprite; // Vehicle base class is assumed to have a sprite
};
