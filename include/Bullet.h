#pragma once
#include <SFML/Graphics.hpp>

enum class BulletType {
    Default,
    Rifle,
    TankShell, 
    Pistol
};

class Bullet {
public:
    Bullet();

    void init(const sf::Vector2f& startPos, const sf::Vector2f& direction, BulletType type = BulletType::Default);
    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons); // ???
    void draw(sf::RenderTarget& target) const;

    bool isActive() const { return m_active; }
    void deactivate() { m_active = false; }
    void setType(BulletType type);
    float getDamage() const { return m_damage; }
    float getExplosionRadius() const { return m_explosionRadius; }
    sf::Vector2f getPosition() const { return m_position; }
    void setActive(bool active) { m_active = active; }


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
