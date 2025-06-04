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

    // תוספת: ניהול זמן לשינוי כיוון
    float timeSinceLastDirectionChange = 0.f;
    const float directionChangeInterval = 2.0f; // שניות

    void setRandomDirection();
};
