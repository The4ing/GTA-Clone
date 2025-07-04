#pragma once

#include "MovingObject.h"
#include "Pedestrian.h"
#include "Vehicle.h"
#include "Player.h"
#include "Present.h"
#include "ResourceManager.h"
#include "CollisionUtils.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include "GameObject.h"

enum class BulletType {
    Default,
    Pistol,
    Rifle,
    TankShell
};

class Bullet : public MovingObject {
public:
    Bullet();
    sf::Vector2f m_startPos; // for debug drawing
    void init(const sf::Vector2f& startPos, const sf::Vector2f& direction, BulletType type = BulletType::Default);
    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) override;
    void draw(sf::RenderTarget& target) override;

    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;
    void move(const sf::Vector2f& moveDir, float dt) override;
    float getSpeed() const override;

    void onCollision(GameObject& other) override;
    void collideWithPlayer(Player& player) override;
    void collideWithPresent(Present& present) override;

    bool checkCollision(const std::vector<std::vector<sf::Vector2f>>& blockedPolygons,
        const std::vector<Pedestrian*>& npcs,
        const std::vector<Vehicle*>& cars);

    void setType(BulletType type);
    bool isActive() const;
    void setActive(bool active);
    float getDamage() const;
    float getExplosionRadius() const;

private:
    sf::Sprite m_sprite;
    sf::Vector2f m_position;
    sf::Vector2f m_direction;
    float m_speed = 500.f;

    BulletType m_type = BulletType::Default;
    float m_damage = 10.f;
    float m_explosionRadius = 0.f;

    bool m_active = false;
};