#pragma once

#include "MovingObject.h" // Or perhaps a more specific base like "AirUnit.h" if one existed
#include <SFML/Graphics.hpp>
#include <vector>

class GameManager; // Forward declaration
class Player;      // Forward declaration

class PoliceHelicopter : public MovingObject {
public:
    PoliceHelicopter(GameManager& gameManager, const sf::Vector2f& startPosition);

    void update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& /*worldBoundaries or obstacles if relevant*/);
    void draw(sf::RenderTarget& target) override;

    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;

    bool isDestroyed() const; // To check if it should be removed
    void takeDamage(int amount);

    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) override;
    void onCollision(GameObject& other) override;
    void collideWithPlayer(Player& player) override;
    void collideWithPresent(Present& present) override;
    void move(const sf::Vector2f& direction, float dt) override;
    float getSpeed() const override;


private:
    void updateAttackBehavior(float dt, Player& player);
    void updateMovement(float dt, const sf::Vector2f& targetPosition);

    GameManager& m_gameManager;
    sf::Sprite m_sprite;
    // sf::Sprite m_rotorSprite; // Optional for animation

    float m_speed = 100.f;
    int m_health = 200; // Helicopters might be tougher
    float m_altitude = 100.f; // Conceptual Z-axis, or just drawn on top

    sf::Vector2f m_targetPosition; // Current movement target (e.g., above player)

    // Attack properties
    float m_fireCooldownTimer = 0.f;
    const float FIRE_RATE = 2.0f; // Seconds between bursts or shots
    const float SHOOTING_RANGE = 300.f; // Effective range
    const float IDEAL_DISTANCE_FROM_TARGET = 250.f; // Distance to try and maintain from player

    // TODO: Add animation manager if complex animations are needed
};
