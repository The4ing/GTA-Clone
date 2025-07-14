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
    float timer = 0.f;
    float frameTimer = 0.f;
    float frameDuration = 0.05f;
    int currentFrame = 0;
    const int sheetCols = 5;
    const int sheetRows = 5;
    int frameWidth = 0;
    int frameHeight = 0;
    float radius = 0.f;

    float duration = 1.f; // ??? ??? ?????? ????
    float m_radius = 1.f; // ???
};
