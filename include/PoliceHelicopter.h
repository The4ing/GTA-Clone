#pragma once

#include "MovingObject.h"
#include <SFML/Graphics.hpp>
#include <vector>

class GameManager;
class Player;

class PoliceHelicopter : public MovingObject {
public:
    PoliceHelicopter(GameManager& gameManager, const sf::Vector2f& startPosition);

    void update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void draw(sf::RenderTarget& target) override;

    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;

    bool isDestroyed() const;
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

    float m_speed = 100.f;
    int m_health = 200;
    float m_altitude = 100.f;

    sf::Vector2f m_targetPosition;

    float m_fireCooldownTimer = 0.f;
    const float FIRE_RATE = 2.0f;
    const float SHOOTING_RANGE = 300.f;
    const float IDEAL_DISTANCE_FROM_TARGET = 250.f;
};
