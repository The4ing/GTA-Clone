#pragma once
#include "GameObject.h"
#include <SFML/Graphics.hpp>

class Explosion : public GameObject {
public:
    Explosion(const sf::Vector2f& pos, float radius);

    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) override;
    void draw(sf::RenderTarget& target) override;

    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;
    void onCollision(GameObject&) override {}
    void collideWithPlayer(Player&) override {}
    void collideWithPresent(Present&) override {}

    bool isFinished() const;

private:
    sf::Sprite sprite;
    sf::Vector2f position;
    float timer ;
    float frameTimer ;
    float frameDuration;
    float duration;
    float m_radius;

    int frameWidth;
    int frameHeight;
    int currentFrame ;
    const int sheetCols;
    const int sheetRows ;
   

    
};
