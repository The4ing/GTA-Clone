#pragma once

#include "MovingObject.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <random>

class GameManager;
class Player;

class PoliceHelicopter : public MovingObject {
public:
    PoliceHelicopter(GameManager& gameManager, const sf::Vector2f& startPosition);
    ~PoliceHelicopter();

    void update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) override;
    void draw(sf::RenderTarget& target) override;
    void move(const sf::Vector2f& direction, float dt) override;

    sf::Vector2f getPosition() const override;
  
    void takeDamage(int amount);
    void startRetreating(const sf::Vector2f& target);

    bool isDestroyed() const;
    bool canSeePlayer(const Player& player, const std::vector<std::vector<sf::Vector2f>>& obstacles);
    
    void onCollision(GameObject& other) override;
    void collideWithPlayer(Player& player) override;
    void collideWithPresent(Present& present) override;

   
    float getSpeed() const override;
    void setPosition(const sf::Vector2f& pos) override;
    bool isRetreating() const;
    bool getNeedsCleanup() const;
    void setNeedsCleanup(bool change);

    //
 

private:
    void updateAttackBehavior(float dt, Player& player);
    void updateMovement(float dt, const sf::Vector2f& targetPosition);
    
    bool needsCleanup;
    bool m_isRetreating;

    GameManager& m_gameManager;
    sf::Sprite m_sprite;
    sf::Vector2f m_targetPosition;

    float m_speed;
    float m_rotationSpeed ;
    int m_health ;
    float m_altitude ;
    float m_fireCooldownTimer ;

   
    sf::Sound m_engineSound;
    std::mt19937 m_rng{ std::random_device{}() };
};
