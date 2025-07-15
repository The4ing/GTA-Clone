#pragma once
#include "MovingObject.h"
#include <SFML/Graphics.hpp>
#include "Constants.h"

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
    bool getBloodSpawned() const;
    void setBloodSpawned(bool val);

    void onCollision(GameObject&) {};
    void collideWithPresent(Present&) {};
    void collideWithPlayer(Player&) {}
    sf::FloatRect getCollisionBounds() const;

private:
    bool checkCollision(const sf::Vector2f& currentPos, const sf::Vector2f& nextPos, 
               const std::vector<std::vector<sf::Vector2f>>& blockedPolygons, float radius);

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
   

   
   
    bool bloodSpawned ;
    bool isBackingUp ;
    bool dying ;
    bool remove ;
    bool isIdle ;
    bool moneyDropped;
   
    void setRandomDirection();
    void updateSprite();
};
