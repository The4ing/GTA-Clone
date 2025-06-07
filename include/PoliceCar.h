#pragma once

#include "MovingObject.h"

class PoliceCar : public MovingObject {
public:
    PoliceCar();

    void update(float dt, const std::vector<sf::Vector2f>& patrolPath);
    void draw(sf::RenderWindow& window);
    void setTarget(const sf::Vector2f& target);

private:
    int         frameWidth;
    int         frameHeight;
    sf::Sprite sprite;
    size_t currentPoint = 0;
    float speed = 80.f;
    std::vector<sf::Vector2f> path;
};
