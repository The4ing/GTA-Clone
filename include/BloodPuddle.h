#pragma once
#include "GameObject.h"
#include <SFML/Graphics.hpp>

class BloodPuddle : public GameObject {
public:
    explicit BloodPuddle(const sf::Vector2f& pos);

    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) override;
    void draw(sf::RenderTarget& target) override;
    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;

    void onCollision(GameObject&) override {}
    void collideWithPlayer(Player&) override {}
    void collideWithPresent(Present&) override {}

private:
    sf::Sprite sprite;
    sf::Vector2f position;
    float frameTimer = 0.f;
    const float frameDuration = 0.1f;
    int currentFrame = 0;
    static constexpr int sheetCols = 3;
    static constexpr int sheetRows = 4;
    int frameWidth = 0;
    int frameHeight = 0;
    bool finished = false;
};