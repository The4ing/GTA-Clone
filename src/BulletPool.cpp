
#include <algorithm> // For std::find_if
#include "BulletPool.h"
BulletPool::BulletPool(size_t initialSize) : maxSize(initialSize) {
    pool.reserve(initialSize);
    for (size_t i = 0; i < initialSize; ++i) {
        auto bullet = std::make_unique<Bullet>(); // Use default constructor
        // Bullet constructor now sets active to false by default
        pool.push_back(std::move(bullet));
    }
}

Bullet* BulletPool::getBullet() {
    // Find an inactive bullet
    for (auto& bullet_ptr : pool) {
        if (!bullet_ptr->isActive()) {
            // No need to call setActive(true) here, that will be done in Bullet::init() or by the caller
            return bullet_ptr.get();
        }
    }

    // Optional: If no inactive bullets, and we want a dynamic pool, create a new one
    // For now, if the pool is exhausted, return nullptr or handle as an error.
    // if (pool.size() < SOME_ABSOLUTE_MAX_CAPACITY) {
    //     auto bullet = std::make_unique<Bullet>(sf::Vector2f(0,0), sf::Vector2f(0,0));
    //     bullet->setActive(false); // Will be activated on init
    //     Bullet* raw_ptr = bullet.get();
    //     pool.push_back(std::move(bullet));
    //     return raw_ptr;
    // }

    return nullptr; // No inactive bullet found and pool is not dynamic or at max capacity
}

void BulletPool::returnBullet(Bullet* bullet) {
    if (bullet) {
        bullet->setActive(false);
        // The bullet remains in the pool, just marked as inactive.
        // No actual removal from the vector needed, which is the point of pooling.
    }
}

std::vector<std::unique_ptr<Bullet>>& BulletPool::getAllBullets() {
    return pool;
}