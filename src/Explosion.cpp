#include "Explosion.h"
#include "ResourceManager.h"

Explosion::Explosion(const sf::Vector2f& pos)
    : position(pos) {
    sprite.setTexture(ResourceManager::getInstance().getTexture("explosion"));
    sprite.setPosition(position);
}

void Explosion::update(float dt) {
    timer += dt;
    
}

void Explosion::draw(sf::RenderTarget& target) {
    if (!isFinished())
        target.draw(sprite);
}

sf::Vector2f Explosion::getPosition() const {
    return position;
}

void Explosion::setPosition(const sf::Vector2f& pos) {
    position = pos;
    sprite.setPosition(pos);
}

bool Explosion::isFinished() const {
    return timer >= duration;
}
