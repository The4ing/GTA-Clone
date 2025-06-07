#pragma once

#include "Character.h"

enum class PoliceState {
    Idle,       
    Chasing     
};

class Police : public Character {
public:
    Police(sf::Vector2f target);

    void update(float dt,  const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void moveToward(const sf::Vector2f& target, float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void draw(sf::RenderWindow& window) ;

    void takeDamage(int amount) ;
    bool isDead() const ;

    float getCollisionRadius() const;
    void setTargetPosition(const sf::Vector2f& pos);

private:
    sf::Vector2f targetPos;
    int     frameWidth;
    int     frameHeight;
    sf::Sprite sprite;
    float speed = 100.f;
    float detectionRadius = 100.f;
    int health = 100;
    PoliceState state = PoliceState::Idle;
    float wanderTimer = 0.f;

    int currentFrame = 0;
    float animationTimer = 0.f;
    float animationSpeed = 0.15f; // כמה שניות בין פריימים
    int animationRow = 1; // 0 = Idle, 1 = Walking (בהתאם לספרייט שלך)


};
