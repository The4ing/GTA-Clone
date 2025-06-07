#include "Character.h"

Character::Character() {}
   

void Character::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    sprite.setPosition(position);
}

void Character::draw(sf::RenderTarget& target) {
    target.draw(sprite);
}

sf::Vector2f Character::getPosition() const {
    return position;
}

void Character::setPosition(const sf::Vector2f& pos) {
   
    sprite.move(pos);
}

void Character::move(const sf::Vector2f& direction, float dt) {
    //position+ = direction * speed * dt;
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

