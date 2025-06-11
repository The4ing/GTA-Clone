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

private:
    sf::Sprite sprite;
    sf::Vector2f position;
    sf::Vector2f direction;
    float speed = 50.f;

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

    bool facingLeft = false;

    void setRandomDirection();
    void updateSprite();
};
