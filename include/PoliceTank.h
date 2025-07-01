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

private:
    void updateAIBehavior(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void updateMovement(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void aimAndFire(Player& player, float dt);

    GameManager& m_gameManager;
    Pathfinder m_pathfinder;
    sf::Sprite m_turretSprite;

    int m_health = 500;
    float m_speed = 30.f;
    float m_rotationSpeed = 45.f;
    float m_turretRotationSpeed = 60.f;

    std::vector<sf::Vector2f> m_currentPath;
    size_t m_currentPathIndex = 0;
    float m_repathTimer = 0.f;
    sf::Vector2f m_targetPosition;

    float m_cannonCooldownTimer = 0.f;
    const float CANNON_FIRE_RATE = 5.0f;
    const float CANNON_RANGE = 400.f;
};
