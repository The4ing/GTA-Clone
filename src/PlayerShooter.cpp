#include "PlayerShooter.h"
#include "GameManager.h"
#include <cmath>

PlayerShooter::PlayerShooter(Player& player, BulletPool& pool)
    : m_player(player), m_pool(pool) {
}

BulletType PlayerShooter::weaponToBullet(const std::string& weapon) const {
    if (weapon == "Pistol") return BulletType::Pistol;
    if (weapon == "Rifle") return BulletType::Rifle;
    if (weapon == "Bazooka") return BulletType::Bazooka;
    return BulletType::Default;
}

// Calculate the muzzle position based on the player's rotation and weapon type
sf::Vector2f PlayerShooter::calculateMuzzlePosition(const std::string& weapon) const {
    sf::Vector2f center = m_player.getCenter();

    // Calculate the rotation in radians
    float angleRad = (m_player.getRotation() - 90.f) * (3.14159f / 180.f); // Convert angle to radians
    sf::Vector2f bulletDir(std::cos(angleRad), std::sin(angleRad)); // Direction vector based on rotation


    float gunOffsetDistance = 5.f;
    sf::Vector2f gunOffset;
    sf::Vector2f bulletStartPos;

    float currentRotation = m_player.getRotation();
    if ((currentRotation >= 315.f && currentRotation <= 360.f) || (currentRotation >= 0.f && currentRotation < 45.f)) {
        // Player is facing up (315° to 45°)
        gunOffset = sf::Vector2f(bulletDir.x, bulletDir.y * 100);
        bulletStartPos = m_player.getCenter();
        bulletStartPos.x += 5;
        bulletStartPos.y += 5;
    }
    else if (
        (currentRotation >= 45.f && currentRotation < 135.f) ||
        (currentRotation >= 135.f && currentRotation < 225.f)
        )
    {
        // Player is facing down (135° to 225°)
        gunOffset = sf::Vector2f(bulletDir.x * gunOffsetDistance, bulletDir.y * gunOffsetDistance);
        bulletStartPos = m_player.getCenter() + gunOffset;
    }
    else {

        gunOffset = sf::Vector2f(bulletDir.x, bulletDir.y * 100);
        bulletStartPos = m_player.getCenter();
        bulletStartPos.x -= 5;
        bulletStartPos.y -= 5;
    }



    // Set weapon-specific muzzle offset
    float weaponOffset = 5.f; // Default muzzle offset for "Pistol"
    if (weapon == "Rifle") {
        weaponOffset = 10.f; // Rifle has a longer muzzle offset
    }
    else if (weapon == "Bazooka") {
        weaponOffset = 15.f; // Bazooka has a much longer muzzle offset
    }
    else if (weapon == "Minigun") {

        weaponOffset = 8.f;


        static float angleOffset = 0.f; 
        angleOffset += 30.f; 

        float radius = 6.f; 
        float angleRadOffset = angleOffset * 3.14159f / 180.f;

        sf::Vector2f circleOffset(std::cos(angleRadOffset), std::sin(angleRadOffset));
        bulletStartPos += circleOffset * radius;
        bulletStartPos.x -= 5;
        bulletStartPos.y -= 5;
        if ((currentRotation >= 135.f && currentRotation < 225.f) || (currentRotation >= 225.f && currentRotation < 335.f)) {
            bulletStartPos.x += 5;
            bulletStartPos.y += 5;
        }

    }
    // Calculate and return the muzzle position based on the offset and direction
    return bulletStartPos;
}

void PlayerShooter::shoot(const std::string& weaponName) {
    if (!m_player.consumeAmmo(weaponName)) {
        SoundManager::getInstance().playSound("noAmmo");
        return;
    }

    Bullet* bullet = m_pool.getBullet();
    if (!bullet) return;

    // Calculate the muzzle position
    sf::Vector2f startPos = calculateMuzzlePosition(weaponName);

    // Calculate the direction for the bullet (same as the player's facing direction)
    float angleRad = (m_player.getRotation() - 90.f) * (3.14159f / 180.f);
    sf::Vector2f dir(std::cos(angleRad), std::sin(angleRad));

    // Initialize the bullet with its starting position and direction
    bullet->init(startPos, dir, weaponToBullet(weaponName), true, false);

    // Play shooting sound
   // SoundManager::getInstance().playSound("gunshot");
}

void PlayerShooter::update(float dt,
    const std::vector<std::vector<sf::Vector2f>>& blockedPolygons,
    const std::vector<Pedestrian*>& npcs,
    const std::vector<Police*>& police,
    const std::vector<Vehicle*>& vehicles,
    Player& player) {
    for (auto& bptr : m_pool.getAllBullets()) {
        if (!bptr->isActive()) continue;
        bptr->update(dt, blockedPolygons);
        bool collided = bptr->checkCollision(blockedPolygons, npcs, police, vehicles, player);
        if (collided && bptr->getExplosionRadius() > 0.f) {
            player.getGameManager().createExplosion(bptr->getPosition(), bptr->getExplosionRadius());
        }
    }
}



