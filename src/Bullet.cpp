#include "Bullet.h"
#include "ResourceManager.h"
#include "CollisionUtils.h"
#include <cmath>
#include <iostream>
Bullet::Bullet() {
}

void Bullet::init(const sf::Vector2f& startPos, const sf::Vector2f& direction,
    BulletType type, bool firedByPlayer, bool ignoreBlocked) {
    m_startPos = startPos;
    m_position = startPos;
    m_direction = direction;
    m_active = true;
    m_firedByPlayer = firedByPlayer;
    m_ignoreBlocked = ignoreBlocked;
    setType(type);
    m_sprite.setPosition(m_position);

    float angleRad = std::atan2(direction.y, direction.x);
    float angleDeg = angleRad * 180.f / 3.14159f;
    m_sprite.setRotation(angleDeg);
}

void Bullet::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    if (!m_active) return;

    sf::Vector2f nextPos = m_position + m_direction * m_speed * dt;
    if (!m_ignoreBlocked && CollisionUtils::isInsideBlockedPolygon(nextPos, blockedPolygons)) {
        m_active = false;
        return;
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
    player.takeDamage(static_cast<int>(m_damage));
    setActive(false);
}

void Bullet::collideWithPresent(Present& present) {
    setActive(false);
}

bool Bullet::checkCollision(const std::vector<std::vector<sf::Vector2f>>& blockedPolygons,
    const std::vector<Pedestrian*>& npcs,
    const std::vector<Police*>& police,
    const std::vector<Vehicle*>& cars,
    Player& player) {
    if (!m_active) return false;

    if (!m_ignoreBlocked && CollisionUtils::isInsideBlockedPolygon(m_position, blockedPolygons)) {
        m_active = false;
        applyExplosionDamage(npcs, police, cars);
        return true;
    }

    for (auto* npc : npcs) {
        if (npc && !npc->isDead() && m_sprite.getGlobalBounds().intersects(npc->getCollisionBounds())) {
            npc->takeDamage(m_damage);
            m_active = false;
            applyExplosionDamage(npcs, police, cars);
            return true;
        }
    }

    for (auto* cop : police) {
        if (m_firedByPlayer && cop && !cop->isDead() && m_sprite.getGlobalBounds().intersects(cop->getCollisionBounds())) {
            cop->takeDamage(static_cast<int>(m_damage));
            m_active = false;
            applyExplosionDamage(npcs, police, cars);
            return true;
        }
    }
    if (!m_firedByPlayer && m_sprite.getGlobalBounds().intersects(player.getCollisionBounds())) {
        player.takeDamage(static_cast<int>(m_damage));
        m_active = false;
        applyExplosionDamage(npcs, police, cars);
        return true;
    }
    for (const auto* car : cars) {
        if (car && m_sprite.getGlobalBounds().intersects(car->getSprite().getGlobalBounds())) {
            m_active = false;
            applyExplosionDamage(npcs, police, cars);
            return true;
        }
    }

    return false;
}

void Bullet::setType(BulletType type) {
    m_type = type;
    sf::Vector2u texSize;
    if (type == BulletType::Default || type == BulletType::Pistol || type == BulletType::Rifle) {
        m_sprite.setTexture(ResourceManager::getInstance().getTexture("bullet_default"));
        texSize = m_sprite.getTexture()->getSize();
        m_sprite.setOrigin(texSize.x / 2.f, texSize.y / 2.f);
        m_sprite.setScale(0.01, 0.01);
        m_damage = 10.f;
        m_explosionRadius = 0.f;
        SoundManager::getInstance().playSound("gunshot");
    }
    else if (type == BulletType::TankShell) {
        std::cout << "tank";
        m_sprite.setTexture(ResourceManager::getInstance().getTexture("bullet_default")); // Consider a unique texture later
        texSize = m_sprite.getTexture()->getSize();
        m_sprite.setOrigin(texSize.x / 2.f, texSize.y / 2.f);
        m_sprite.setScale(0.1f, 0.05f); // Make it larger and somewhat shell-shaped
        m_damage = 100.f;
        m_explosionRadius = 100.f;
        // Consider adding SoundManager::getInstance().playSound("tank_fire_sound");
    }
    else if (type == BulletType::Minigun) {
        m_sprite.setTexture(ResourceManager::getInstance().getTexture("bullet_default"));
        texSize = m_sprite.getTexture()->getSize();
        m_sprite.setOrigin(texSize.x / 2.f, texSize.y / 2.f);
        m_damage = 100.f;
        m_explosionRadius = 100.f;
        std::cout << "Minigun";
    }
    else if (type == BulletType::Bazooka) {
        m_sprite.setTexture(ResourceManager::getInstance().getTexture("bullet_RPG"));
        texSize = m_sprite.getTexture()->getSize();
        m_sprite.setOrigin(texSize.x / 2.f, texSize.y / 2.f);
        std::cout << "bazooka";
        m_sprite.setScale(0.05, 0.05);
        m_damage = 100.f;
        m_explosionRadius = 100.f;
        SoundManager::getInstance().playSound("RPGshot");
    }
    else if (type == BulletType::Grenade) {

        m_damage = 100.f;
        m_explosionRadius = 100.f;
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


void Bullet::applyExplosionDamage(const std::vector<Pedestrian*>& npcs,
    const std::vector<Police*>& police,
    const std::vector<Vehicle*>& cars) {
    if (m_explosionRadius <= 0.f)
        return;

    for (auto* npc : npcs) {
        if (!npc)
            continue;
        float dx = npc->getPosition().x - m_position.x;
        float dy = npc->getPosition().y - m_position.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist <= m_explosionRadius) {
            float factor = 1.f - (dist / m_explosionRadius);
            npc->takeDamage(m_damage * factor);
        }
    }

    for (auto* cop : police) {
        if (!cop)
            continue;
        if (!m_firedByPlayer)
            continue;
        float dx = cop->getPosition().x - m_position.x;
        float dy = cop->getPosition().y - m_position.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist <= m_explosionRadius) {
            float factor = 1.f - (dist / m_explosionRadius);
            cop->takeDamage(static_cast<int>(m_damage * factor));
        }
    }

    (void)cars; // vehicles currently ignore damage
}