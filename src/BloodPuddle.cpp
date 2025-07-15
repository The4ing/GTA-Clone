#include "BloodPuddle.h"
#include "ResourceManager.h"

BloodPuddle::BloodPuddle(const sf::Vector2f& pos) : position(pos) {
    sprite.setTexture(ResourceManager::getInstance().getTexture("blood_puddle"));
    sf::Vector2u texSize = sprite.getTexture()->getSize();
    frameWidth = texSize.x / sheetCols;
    frameHeight = texSize.y / sheetRows;
    sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
    sprite.setScale(0.3, 0.3);
    sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));
    sprite.setPosition(position);
}

void BloodPuddle::update(float dt, const std::vector<std::vector<sf::Vector2f>>& /*blockedPolygons*/) {
    if (finished)
        return;
    frameTimer += dt;
    if (frameTimer >= frameDuration) {
        frameTimer -= frameDuration;
        currentFrame++;
        if (currentFrame >= 7) {
            currentFrame = 6; // stay on last frame
            finished = true;
        }
        int row = currentFrame / sheetCols;
        int col = currentFrame % sheetCols;
        sprite.setTextureRect(sf::IntRect(col * frameWidth, row * frameHeight, frameWidth, frameHeight));
    }
}

void BloodPuddle::draw(sf::RenderTarget& target) {
    target.draw(sprite);
}

sf::Vector2f BloodPuddle::getPosition() const {
    return position;
}

void BloodPuddle::setPosition(const sf::Vector2f& pos) {
    position = pos;
    sprite.setPosition(pos);
}