#include "Bullet.h"
#include "ResourceManager.h"

Bullet::Bullet(const sf::Vector2f& start, const sf::Vector2f& dir)
    : position(start), direction(dir) {
    sprite.setTexture(ResourceManager::getInstance().getTexture("bullet"));
    sprite.setPosition(position);
}

void Bullet::update(float dt) {
    move(direction, dt);
    sprite.setPosition(position);
}

void Bullet::draw(sf::RenderTarget& target) {
    target.draw(sprite);
}

sf::Vector2f Bullet::getPosition() const {
    return position;
}

void Bullet::setPosition(const sf::Vector2f& pos) {
    position = pos;
    sprite.setPosition(position);
}

void Bullet::move(const sf::Vector2f& dir, float dt) {
    position += dir * speed * dt;
}

float Bullet::getSpeed() const {
    return speed;
}
