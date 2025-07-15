#include "Character.h"

Character::Character():speed(100.f), health(100)
{}
   

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
    this->position = pos;
    this->sprite.setPosition(this->position); 
}

void Character::move(const sf::Vector2f& direction, float dt) {
    this->position += direction * this->speed * dt; // Update member variable
    this->sprite.setPosition(this->position);     // Update sprite position
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

