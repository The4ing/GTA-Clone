#pragma once
#include "MovingObject.h"
#include <SFML/Graphics.hpp>

class Character : public MovingObject {
public:
    Character();
    virtual ~Character() = default;

    virtual void update(float dt) override;
    virtual void draw(sf::RenderTarget& target) override;
    virtual sf::Vector2f getPosition() const override;
    virtual void setPosition(const sf::Vector2f& pos) override;

    virtual void move(const sf::Vector2f& direction, float dt) override;
    virtual float getSpeed() const override;

    void takeDamage(int amount);
    bool isDead() const;

protected:
    sf::Sprite sprite;
    sf::Vector2f position;
    float speed = 100.f;
    int health = 100;
};
