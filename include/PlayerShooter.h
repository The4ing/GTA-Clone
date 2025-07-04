#pragma once

#include "SoundManager.h"
#include "BulletPool.h"
#include "Player.h"
#include <string>

class PlayerShooter {
public:
    PlayerShooter(Player& player, BulletPool& pool);

    // Attempt to fire the player's current weapon
    void shoot(const std::string& weaponName);

    // Update all bullets managed by the pool and handle collisions
    void update(float dt,
        const std::vector<std::vector<sf::Vector2f>>& blockedPolygons,
        const std::vector<Pedestrian*>& npcs,
        const std::vector<Vehicle*>& vehicles);


private:
    BulletType weaponToBullet(const std::string& weapon) const;
    sf::Vector2f calculateMuzzlePosition() const;

    Player& m_player;
    BulletPool& m_pool;
    // ammo information resides in Player
};
