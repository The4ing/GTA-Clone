#pragma once
#include "MovingObject.h"
#include <SFML/Graphics.hpp>

class Character : public MovingObject {
public:
    Character();
    virtual ~Character() = default;

    virtual void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) override;
    virtual void draw(sf::RenderTarget& target) override;
    virtual void move(const sf::Vector2f& direction, float dt) override;
    virtual void setPosition(const sf::Vector2f& pos) override;
    virtual void takeDamage(int amount);

    virtual sf::Vector2f getPosition() const override;

    virtual float getSpeed() const override;

    virtual bool isDead() const;

private:
    sf::Sprite sprite;
    sf::Vector2f position;
    float speed ;
    int health;
};