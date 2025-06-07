#pragma once
#include "GameObject.h"
#include <SFML/Graphics.hpp>

class DestructibleObject : public GameObject {
public:
    DestructibleObject();
    virtual ~DestructibleObject() = default;

    virtual void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) override;
    virtual void draw(sf::RenderTarget& target) override;
    virtual sf::Vector2f getPosition() const override;
    virtual void setPosition(const sf::Vector2f& pos) override;

    void takeDamage(int amount);
    bool isDestroyed() const;

protected:
    sf::Sprite sprite;
    sf::Vector2f position;
    int health = 100;
};
