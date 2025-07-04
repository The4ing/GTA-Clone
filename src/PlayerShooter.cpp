#include "PlayerShooter.h"
#include <cmath>

PlayerShooter::PlayerShooter(Player& player, BulletPool& pool)
    : m_player(player), m_pool(pool) {
}


BulletType PlayerShooter::weaponToBullet(const std::string& weapon) const {
    if (weapon == "Pistol") return BulletType::Pistol;
    if (weapon == "Rifle") return BulletType::Rifle;
    if (weapon == "Bazooka") return BulletType::TankShell;
    return BulletType::Default;
}

sf::Vector2f PlayerShooter::calculateMuzzlePosition() const {
    sf::Vector2f center = m_player.getCenter();
    float angleRad = (m_player.getRotation() - 90.f) * (3.14159f / 180.f);
    sf::Vector2f dir(std::cos(angleRad), std::sin(angleRad));
    return center + dir * 5.f;
}

void PlayerShooter::shoot(const std::string& weaponName) {
    if (!m_player.consumeAmmo(weaponName)) {
        SoundManager::getInstance().playSound("noAmmo");
        return;
    }

    Bullet* bullet = m_pool.getBullet();
    if (!bullet) return;

    sf::Vector2f startPos = calculateMuzzlePosition();
    float angleRad = (m_player.getRotation() - 90.f) * (3.14159f / 180.f);
    sf::Vector2f dir(std::cos(angleRad), std::sin(angleRad));

    bullet->init(startPos, dir, weaponToBullet(weaponName));
    SoundManager::getInstance().playSound("gunshot");
}

void PlayerShooter::update(float dt,
    const std::vector<std::vector<sf::Vector2f>>& blockedPolygons,
    const std::vector<Pedestrian*>& npcs,
    const std::vector<Vehicle*>& vehicles) {
    for (auto& bptr : m_pool.getAllBullets()) {
        if (!bptr->isActive()) continue;
        bptr->update(dt, blockedPolygons);
        if (bptr->checkCollision(blockedPolygons, npcs, vehicles)) {
            SoundManager::getInstance().playSound("gunshot");
        }
    }
}
