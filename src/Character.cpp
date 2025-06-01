#include "Character.h"

Character::Character() : position(0.f, 0.f) {
    sprite.setPosition(position);
}

void Character::update(float dt) {
    sprite.setPosition(position);
}

void Character::draw(sf::RenderTarget& target) {
    target.draw(sprite);
}

sf::Vector2f Character::getPosition() const {
    return position;
}

void Character::setPosition(const sf::Vector2f& pos) {
    position = pos;
    sprite.setPosition(position);
}

void Character::move(const sf::Vector2f& direction, float dt) {
    position += direction * speed * dt;
    sprite.setPosition(position);
}

float Character::getSpeed() const {
    return speed;
}

void Character::takeDamage(int amount) {
    health -= amount;
    if (health < 0) health = 0;
}

bool Character::isDead() const {
    return health <= 0;
}
