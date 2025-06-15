#include "Present.h"

Present::Present(const sf::Texture& texture, const sf::Vector2f& pos) {
    sprite.setTexture(texture);
    sprite.setScale(0.1f, 0.1f);
    setPosition(pos);
}

void Present::update(float /*dt*/, const std::vector<std::vector<sf::Vector2f>>& /*blockedPolygons*/) {
    // No animation for now
}

void Present::draw(sf::RenderTarget& target) {
    if (!collected)
        target.draw(sprite);
}

sf::Vector2f Present::getPosition() const {
    return position;
}

void Present::setPosition(const sf::Vector2f& pos) {
    position = pos;
    sprite.setPosition(pos);
}

void Present::collect() {
    collected = true;
}

bool Present::isCollected() const {
    return collected;
}

sf::Sprite& Present::getSprite() {
    return sprite;
}

const sf::Sprite& Present::getSprite() const {
    return sprite;
}

void Present::setTexture(const sf::Texture& texture) {
    sprite.setTexture(texture);
}
