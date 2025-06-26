#pragma once
#include "MovingObject.h"
#include "Pedestrian.h"
#include "Vehicle.h"
#include "Player.h"
#include "Present.h"

class Bullet : public MovingObject {
public:
    Bullet();
    void init(const sf::Vector2f& startPos, const sf::Vector2f& dir, float initialSpeed = 600.f);

    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) override;
    void draw(sf::RenderTarget& target) override;

    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;
    void move(const sf::Vector2f& moveDir, float dt) override; // Parameter renamed for clarity
    float getSpeed() const override;

    // GameObject pure virtual overrides:
    void onCollision(GameObject& other) override;
    void collideWithPlayer(Player& player) override;
    void collideWithPresent(Present& present) override;

    bool checkCollision(const std::vector<std::vector<sf::Vector2f>>& blockedPolygons,
        const std::vector<Pedestrian>& npcs,
        const std::vector<Vehicle>& cars);

    bool isActive() const { return active; }
    void setActive(bool isActive) { active = isActive; }

private:
    bool active = true;
    sf::Sprite sprite;
    sf::Vector2f position;
    sf::Vector2f direction;
    float speed = 600.f;
};
