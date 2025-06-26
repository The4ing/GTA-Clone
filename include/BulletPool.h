#pragma once

#include "Bullet.h"
#include <vector>
#include <memory>

class BulletPool {
public:
    BulletPool(size_t initialSize = 50); // Constructor with an initial size

    Bullet* getBullet(); // Get an inactive bullet from the pool
    void returnBullet(Bullet* bullet); // Return a bullet to the pool

    // Method to update active bullets (bullets themselves know if they are active)
    // GameManager will iterate this list for update and draw
    std::vector<std::unique_ptr<Bullet>>& getAllBullets();


private:
    std::vector<std::unique_ptr<Bullet>> pool;
    size_t maxSize; // Max size of the pool
};