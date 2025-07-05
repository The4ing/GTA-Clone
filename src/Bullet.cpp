#include "Bullet.h"
#include "ResourceManager.h"
#include "CollisionUtils.h"
#include <cmath>
#include <iostream>
Bullet::Bullet() {
}

void Bullet::init(const sf::Vector2f& startPos, const sf::Vector2f& direction, BulletType type) {
    m_startPos = startPos;
    m_position = startPos;
    m_direction = direction;
    m_active = true;
    setType(type);
    m_sprite.setPosition(m_position);

    float angleRad = std::atan2(direction.y, direction.x);
    float angleDeg = angleRad * 180.f / 3.14159f;
    m_sprite.setRotation(angleDeg);
}

void Bullet::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    if (!m_active) return;

    sf::Vector2f nextPos = m_position + m_direction * m_speed * dt;
    for (const auto& polygon : blockedPolygons) {
        if (CollisionUtils::pointInPolygon(nextPos, polygon)) {
            m_active = false;
            return;
        }
    }

    m_position = nextPos;
    m_sprite.setPosition(m_position);
}

void Bullet::draw(sf::RenderTarget& target) {
    if (m_active)
        target.draw(m_sprite);
    // Debug: draw line showing bullet trajectory
    sf::VertexArray trajectory(sf::Lines, 2);
    trajectory[0].position = m_startPos;
    trajectory[0].color = sf::Color::Red;
    trajectory[1].position = m_position + m_direction * 1000.f; // arbitrary long line
    trajectory[1].color = sf::Color::Red;

    target.draw(trajectory);
}

sf::Vector2f Bullet::getPosition() const {
    return m_position;
}

void Bullet::setPosition(const sf::Vector2f& pos) {
    m_position = pos;
    m_sprite.setPosition(pos);
}

void Bullet::move(const sf::Vector2f& dir, float dt) {
    m_position += dir * m_speed * dt;
    m_sprite.setPosition(m_position);
}

float Bullet::getSpeed() const {
    return m_speed;
}

void Bullet::onCollision(GameObject& other) {
    if (auto* player = dynamic_cast<Player*>(&other)) collideWithPlayer(*player);
    else if (auto* present = dynamic_cast<Present*>(&other)) collideWithPresent(*present);
    m_active = false;
}

void Bullet::collideWithPlayer(Player& player) {
    setActive(false);
}

void Bullet::collideWithPresent(Present& present) {
    setActive(false);
}

bool Bullet::checkCollision(const std::vector<std::vector<sf::Vector2f>>& blockedPolygons,
    const std::vector<Pedestrian*>& npcs,
    const std::vector<Vehicle*>& cars) {
    if (!m_active) return false;

    for (const auto& polygon : blockedPolygons) {
        if (CollisionUtils::pointInPolygon(m_position, polygon)) {
            m_active = false;
            return true;
        }
    }

    for (const auto* npc : npcs) {
        if (npc && m_sprite.getGlobalBounds().intersects(npc->getCollisionBounds())) {
            m_active = false;
            return true;
        }
    }

    for (const auto* car : cars) {
        if (car && m_sprite.getGlobalBounds().intersects(car->getSprite().getGlobalBounds())) {
            m_active = false;
            return true;
        }
    }

    return false;
}

void Bullet::setType(BulletType type) {//??? ??? ?????########################################################################################################
    m_type = type;
    sf::Vector2u texSize;
    switch (type) {
    case BulletType::Default:
        m_sprite.setTexture(ResourceManager::getInstance().getTexture("bullet_default"));
        texSize = m_sprite.getTexture()->getSize();
        m_sprite.setOrigin(texSize.x / 2.f, texSize.y / 2.f);
        m_sprite.setScale(0.01, 0.01);
        m_damage = 10.f;
        m_explosionRadius = 0.f;
        break;
    case BulletType::Pistol:
        m_sprite.setTexture(ResourceManager::getInstance().getTexture("bullet_default"));
        texSize = m_sprite.getTexture()->getSize();
        m_sprite.setOrigin(texSize.x / 2.f, texSize.y / 2.f);
        m_sprite.setScale(0.01, 0.01);
        m_damage = 10.f;
        m_explosionRadius = 0.f;
        break;
    case BulletType::Rifle:
        std::cout << "bazooka";
        m_sprite.setTexture(ResourceManager::getInstance().getTexture("bullet_default"));
        texSize = m_sprite.getTexture()->getSize();
        m_sprite.setOrigin(texSize.x / 2.f, texSize.y / 2.f);
        m_sprite.setScale(0.01, 0.01);
        m_damage = 10.f;
        m_explosionRadius = 0.f;
        break;
    case BulletType::TankShell:
        std::cout << "tank";
        m_sprite.setTexture(ResourceManager::getInstance().getTexture("bullet_default"));
        texSize = m_sprite.getTexture()->getSize();
        m_sprite.setOrigin(texSize.x / 2.f, texSize.y / 2.f);
        m_damage = 100.f;
        m_explosionRadius = 100.f;
        break;
    case BulletType::Minigun:
        std::cout << "bazooka";
     //   m_sprite.setTexture(ResourceManager::getInstance().getTexture("bullet_default"));
    //    m_damage = 100.f;
    //    m_explosionRadius = 100.f;
        break;
    case BulletType::Bazooka:
        m_sprite.setTexture(ResourceManager::getInstance().getTexture("bullet_RPG"));
        texSize = m_sprite.getTexture()->getSize();
        m_sprite.setOrigin(texSize.x / 2.f, texSize.y / 2.f);
        std::cout << "bazooka";
        m_sprite.setScale(0.05, 0.05);
        m_damage = 100.f;
        m_explosionRadius = 100.f;
        break;
    case BulletType::Grenade:
        m_sprite.setTexture(ResourceManager::getInstance().getTexture("bullet_default"));
        std::cout << "bazooka";
        m_damage = 100.f;
        m_explosionRadius = 100.f;
        break;
    }

}

bool Bullet::isActive() const {
    return m_active;
}

void Bullet::setActive(bool active) {
    m_active = active;
}

float Bullet::getDamage() const {
    return m_damage;
}

float Bullet::getExplosionRadius() const {
    return m_explosionRadius;
}
