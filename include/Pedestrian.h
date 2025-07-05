#pragma once
#include "MovingObject.h"
#include <SFML/Graphics.hpp>

class Pedestrian : public MovingObject {
public:
    Pedestrian(sf::Vector2f pos);

    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) override;
    void draw(sf::RenderTarget& target) override;

    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;
    void move(const sf::Vector2f& direction, float dt) override;
    float getSpeed() const override;
    float getCollisionRadius() const;
    void takeDamage(float amount);
    bool isDead() const;
    bool shouldRemove() const { return remove; }

    void onCollision(GameObject&) {};
    void collideWithPresent(Present&) {};
    void collideWithPlayer(Player&) {}
    sf::FloatRect getCollisionBounds() const;

    void startBackingUp();
    bool getIsBackingUp() const;

private:
    bool checkCollision(const sf::Vector2f& currentPos, const sf::Vector2f& nextPos, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons, float radius);

    sf::Sprite sprite;
    sf::Vector2f position;

    sf::Vector2f direction;        
    sf::Vector2f nextDirection;   

    float speed = 50.f;
    int health = 100;
    int characterRow = 0;
    int currentFrame = 0;
    float animationTimer = 0.f;
    const float animationSpeed = 0.12f;

    float timeSinceLastDirectionChange = 0.f;
    const float directionChangeInterval = 2.0f;

    static constexpr int frameWidth = 64;
    static constexpr int frameHeight = 64;
    static constexpr int framesPerRow = 3;
    static constexpr int numCharacters = 7;

    const float backupDistance = 30.f;
    bool isBackingUp = false;
    float backupProgress = 0.f;
    bool dying = false;
    float deathTimer = 0.f;
    const float deathDuration = 3.f;
    bool remove = false;


    bool isIdle = false;
    float idleTimer = 0.f;
    float idleDurationMin = 1.f;
    float idleDurationMax = 3.f;
    float idleProbability = 0.15f;

    void setRandomDirection();
    void updateSprite();
};
