#include "Vehicle.h"
#include "ResourceManager.h"

Vehicle::Vehicle() {
    sprite.setTexture(ResourceManager::getInstance().getTexture("car"));
    position = { 300.f, 300.f };
    sprite.setPosition(position);
}

void Vehicle::update(float dt) {
    sprite.setPosition(position);
}

void Vehicle::draw(sf::RenderTarget& target) {
    target.draw(sprite);
}

sf::Vector2f Vehicle::getPosition() const {
    return position;
}

void Vehicle::setPosition(const sf::Vector2f& pos) {
    position = pos;
    sprite.setPosition(pos);
}

void Vehicle::move(const sf::Vector2f& direction, float dt) {
    position += direction * speed * dt;
}

float Vehicle::getSpeed() const {
    return speed;
}
