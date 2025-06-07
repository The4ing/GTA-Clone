#pragma once
#include "MovingObject.h"

class Bullet : public MovingObject {
public:
    Bullet(const sf::Vector2f& start, const sf::Vector2f& dir);

    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) override;
    void draw(sf::RenderTarget& target) override;

    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;

    void move(const sf::Vector2f& direction, float dt) override;
    float getSpeed() const override;

private:
    sf::Sprite sprite;
    sf::Vector2f position;
    sf::Vector2f direction;
    float speed = 600.f;
};
