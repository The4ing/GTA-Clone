#pragma once
#include "MovingObject.h"
#include <SFML/Graphics.hpp>

class Pedestrian : public MovingObject {
public:
    Pedestrian();
    void update(float dt) override;
    void draw(sf::RenderTarget& target) override;

    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;

    void move(const sf::Vector2f& direction, float dt) override;
    float getSpeed() const override;

private:
    sf::Sprite sprite;
    sf::Vector2f position;
    sf::Vector2f direction;
    float speed = 50.f;

    float timeSinceLastDirectionChange = 0.f;
    const float directionChangeInterval = 2.0f; // שניות

    // Sprite sheet details
    sf::IntRect spriteRect;
    const int frameWidth = 341;
    const int frameHeight = 512;
    const int maxWalkFrames = 3;


    int currentFrame = 0;
    float animationTimer = 0.f;
    const float animationSpeed = 0.12f; // כל 0.1 שניות מחליפים פריים
    int currentRow = 0; // 0 = קדימה, 1 = צד, 2 = אחורה

    //when he will be dead
    bool isDead = false;
    void die();

    void setRandomDirection();
    void updateSpriteDirection();
   
};
