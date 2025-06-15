#pragma once
#include "GameObject.h"

class MovingObject : public GameObject {
public:
    virtual ~MovingObject() = default;

    virtual void move(const sf::Vector2f& direction, float dt) = 0;
    virtual float getSpeed() const = 0;
    //virtual bool circleIntersectsPolygon(const sf::Vector2f& circleCenter, float radius, const std::vector<sf::Vector2f>& polygon);
};
