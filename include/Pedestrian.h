#pragma once
#include "MovingObject.h"
#include <SFML/Graphics.hpp>

class Pedestrian : public MovingObject {
public:
    Pedestrian(sf::Vector2f pos);

    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) override;
    void draw(sf::RenderTarget& target) override;
    void move(const sf::Vector2f& direction, float dt) override;

    void takeDamage(float amount);
    void startBackingUp();
   
    float getSpeed() const override;
    float getCollisionRadius() const;

    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;
   
    bool isDead() const;
    bool shouldRemove() const;
    bool getIsBackingUp() const;
    bool getMoneyDropped() const;
    void setMoneyDropped(bool change);
    bool getBloodSpawned() const { return bloodSpawned; }
    void setBloodSpawned(bool val) { bloodSpawned = val; }

    void onCollision(GameObject&) {};
    void collideWithPresent(Present&) {};
    void collideWithPlayer(Player&) {}
    sf::FloatRect getCollisionBounds() const;

private:
    bool checkCollision(const sf::Vector2f& currentPos, const sf::Vector2f& nextPos, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons, float radius);

    sf::Sprite sprite;
    sf::Vector2f position;

    sf::Vector2f direction;        
    sf::Vector2f nextDirection;   

    float idleTimer;
    float idleDurationMin ;
    float idleDurationMax;
    float idleProbability ;
    float speed ;
    float animationTimer ;
    float timeSinceLastDirectionChange ;
    float backupProgress ;
    float deathTimer ;
    int health ;
    int characterRow ;
    int currentFrame;
   

    const float animationSpeed = 0.12f;
    const float directionChangeInterval = 2.0f;
    const float backupDistance = 30.f;
    const float deathDuration = 3.f;

    static constexpr int frameWidth = 64;
    static constexpr int frameHeight = 64;
    static constexpr int framesPerRow = 3;
    static constexpr int numCharacters = 7;
    bool bloodSpawned = false;
   
    bool isBackingUp ;
    bool dying ;
    bool remove ;
    bool isIdle ;
    bool moneyDropped;
   
    void setRandomDirection();
    void updateSprite();
};
