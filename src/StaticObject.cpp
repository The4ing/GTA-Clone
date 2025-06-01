#include "StaticObject.h"

StaticObject::StaticObject() : position(0.f, 0.f) {
    sprite.setPosition(position);
}

void StaticObject::update(float) {
    //nothing
}

void StaticObject::draw(sf::RenderTarget& target) {
    target.draw(sprite);
}

sf::Vector2f StaticObject::getPosition() const {
    return position;
}

void StaticObject::setPosition(const sf::Vector2f& pos) {
    position = pos;
    sprite.setPosition(pos);
}
