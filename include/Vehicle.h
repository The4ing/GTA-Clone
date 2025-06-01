#pragma once
#include "MovingObject.h"

class Vehicle : public MovingObject {
public:
    Vehicle();
    void update(float dt) override;
    void draw(sf::RenderTarget& target) override;

    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;

    void move(const sf::Vector2f& direction, float dt) override;
    float getSpeed() const override;

private:
    sf::Sprite sprite;
    sf::Vector2f position;
    float speed = 250.f;
};
