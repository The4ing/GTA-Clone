#include "Present.h"

Present::Present(const sf::Texture& texture, const sf::Vector2f& pos) {
    sprite.setTexture(texture);
    sprite.setScale(0.02f, 0.02f);

    sf::FloatRect bounds = sprite.getLocalBounds();
    sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    setPosition(pos);
}

void Present::update(float dt, const std::vector<std::vector<sf::Vector2f>>& /*blockedPolygons*/) {
    if (collected) {
        if (respawnTimer > 0.f)
            respawnTimer -= dt;
        if (respawnTimer <= 0.f)
            collected = false; 
    }
    else {
        sprite.rotate(50.f * dt);
    }
}


void Present::draw(sf::RenderTarget& target) {
    if (!collected)
        target.draw(sprite);
}

void Present::onCollision(GameObject& other) {
    other.collideWithPresent(*this); // ← אתה מתנה, שולח את עצמך
}

void Present::collideWithPlayer(Player& player) {
    if (!collected && respawnTimer <= 0.f) {
        applyEffect(player);
       
        respawnTimer = 30.f;
    }
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