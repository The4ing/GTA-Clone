#pragma once
#include "GameObject.h"

class StaticObject : public GameObject {
public:
    StaticObject();
    virtual ~StaticObject() = default;

    virtual void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons)  override;
    virtual void draw(sf::RenderTarget& target) override;
    virtual sf::Vector2f getPosition() const override;
    virtual void setPosition(const sf::Vector2f& pos) override;

protected:
    sf::Sprite sprite;
    sf::Vector2f position;
};
