#include "DestructibleObject.h"
#include "ResourceManager.h"

DestructibleObject::DestructibleObject() {
    sprite.setTexture(ResourceManager::getInstance().getTexture("barrel"));
    position = { 400.f, 300.f };
    sprite.setPosition(position);
}

void DestructibleObject::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    //nothing
}

void DestructibleObject::draw(sf::RenderTarget& target) {
    if (!isDestroyed())
        target.draw(sprite);
}

sf::Vector2f DestructibleObject::getPosition() const {
    return position;
}

void DestructibleObject::setPosition(const sf::Vector2f& pos) {
    position = pos;
    sprite.setPosition(pos);
}

void DestructibleObject::takeDamage(int amount) {
    health -= amount;
    if (health < 0) health = 0;
}

bool DestructibleObject::isDestroyed() const {
    return health <= 0;
}
