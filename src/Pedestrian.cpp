#include "Pedestrian.h"
#include "ResourceManager.h"
#include <cstdlib>

Pedestrian::Pedestrian() {
    sprite.setTexture(ResourceManager::getInstance().getTexture("pedestrian"));
    position = { static_cast<float>(rand() % 800), static_cast<float>(rand() % 600) };
    sprite.setPosition(position);

    direction = {
        static_cast<float>((rand() % 3) - 1), // -1, 0, 1
        static_cast<float>((rand() % 3) - 1)
    };
}

void Pedestrian::update(float dt) {
    move(direction, dt);
    sprite.setPosition(position);
}

void Pedestrian::draw(sf::RenderTarget& target) {
    target.draw(sprite);
}

sf::Vector2f Pedestrian::getPosition() const {
    return position;
}

void Pedestrian::setPosition(const sf::Vector2f& pos) {
    position = pos;
    sprite.setPosition(pos);
}

void Pedestrian::move(const sf::Vector2f& dir, float dt) {
    position += dir * speed * dt;
}

float Pedestrian::getSpeed() const {
    return speed;
}
