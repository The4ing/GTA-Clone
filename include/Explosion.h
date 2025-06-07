#pragma once
#include "GameObject.h"
#include <SFML/Graphics.hpp>

class Explosion : public GameObject {
public:
    Explosion(const sf::Vector2f& pos);

    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) override;
    void draw(sf::RenderTarget& target) override;

    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;

    bool isFinished() const;

private:
    sf::Sprite sprite;
    sf::Vector2f position;
    float timer = 0.f;
    float duration = 0.5f; 
};
