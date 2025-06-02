#pragma once
#include "Character.h"
#include <SFML/Graphics.hpp>

class Player : public Character {
public:
    Player();

    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition() const;
    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void draw(sf::RenderWindow& window);
    sf::FloatRect getCollisionBounds(const sf::Vector2f& offset = {}) const;
    sf::Vector2f getCenter() const;
    float getCollisionRadius() const;
    bool circleIntersectsPolygon(const sf::Vector2f& circleCenter, float radius, const std::vector<sf::Vector2f>& polygon);


private:

    sf::Sprite  sprite;
    float       speed = 50.f;
    int         frameWidth;
    int         frameHeight;
    int         currentFrame;
    int         sheetCols;
    int         sheetRows;
    float       animTimer;
    float       animDelay;
};
