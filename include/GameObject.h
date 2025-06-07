#pragma once
#include <SFML/Graphics.hpp>

class GameObject {
public:
    virtual ~GameObject() = default;

    virtual void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) = 0;
    virtual void draw(sf::RenderTarget& target) = 0;
    virtual sf::Vector2f getPosition() const = 0;
    virtual void setPosition(const sf::Vector2f& pos) = 0;
};
