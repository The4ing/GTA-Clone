#pragma once
#include <SFML/Graphics.hpp>

class Player;
class Present;

class GameObject {
public:
    virtual ~GameObject() = default;

    virtual void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) = 0;
    virtual void draw(sf::RenderTarget& target) = 0;
    virtual sf::Vector2f getPosition() const = 0;
    virtual void setPosition(const sf::Vector2f& pos) = 0;

    virtual void onCollision(GameObject& other) = 0;

    // ← נוסיף את שתי הפונקציות החשובות:
    virtual void collideWithPlayer(Player& player) = 0;
    virtual void collideWithPresent(Present& present) = 0;
};
