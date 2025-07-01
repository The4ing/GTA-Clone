#include "Bullet.h"
#include "ResourceManager.h"
#include <cmath>
#include "CollisionUtils.h" // ??? ??? ????

Bullet::Bullet() {
    m_sprite.setOrigin(8.f, 8.f); // Adjust as needed to center the bullet sprite
}

void Bullet::init(const sf::Vector2f& startPos, const sf::Vector2f& direction, BulletType type) {
    m_position = startPos;
    m_direction = direction;
    m_active = true;
    setType(type);
    m_sprite.setPosition(m_position);
}

void Bullet::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    if (!m_active) return;

    sf::Vector2f nextPos = m_position + m_direction * m_speed * dt;

    // ???? ??????? ?? ???????
    for (const auto& polygon : blockedPolygons) {
        if (CollisionUtils::pointInPolygon(nextPos, polygon)) {
            m_active = false;
            return;
        }
    }

    m_position = nextPos;
    m_sprite.setPosition(m_position);
}


void Bullet::draw(sf::RenderTarget& target) const {
    if (m_active)
        target.draw(m_sprite);
}

void Bullet::setType(BulletType type) {
    m_type = type;

    switch (type) {
    case BulletType::Default:
        m_sprite.setTexture(ResourceManager::getInstance().getTexture("bullet_default"));
        m_damage = 10.f;
        m_explosionRadius = 0.f;
        break;

    case BulletType::Pistol:
        m_sprite.setTexture(ResourceManager::getInstance().getTexture("bullet_pistol"));
        m_damage = 10.f;
        m_explosionRadius = 0.f;
        break;

    case BulletType::Rifle:
        m_sprite.setTexture(ResourceManager::getInstance().getTexture("bullet_rifle"));
        m_damage = 10.f;
        m_explosionRadius = 0.f;
        break;

    case BulletType::TankShell:
        m_sprite.setTexture(ResourceManager::getInstance().getTexture("bullet_tank"));
        m_damage = 100.f;
        m_explosionRadius = 100.f;
        break;
    }
}
