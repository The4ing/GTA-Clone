#include "Explosion.h"
#include "ResourceManager.h"
#include <cassert>

Explosion::Explosion(const sf::Vector2f& pos, float radius)
    : position(pos), m_radius(radius)
{
    assert(radius > 0.f);  // ???? ?????? ?? ?? ?? ??? ???? ?????? ????
    sprite.setTexture(ResourceManager::getInstance().getTexture("explosion"));
    sf::Vector2u texSize = sprite.getTexture()->getSize();
    frameWidth = texSize.x / sheetCols;
    frameHeight = texSize.y / sheetRows;
    sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
    sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));

    float scaleX = (radius * 2.f) / static_cast<float>(frameWidth);
    float scaleY = (radius * 2.f) / static_cast<float>(frameHeight);
    sprite.setScale(scaleX, scaleY);
    sprite.setPosition(position);
}

void Explosion::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    (void)blockedPolygons;
    timer += dt;
    frameTimer += dt;
    if (frameTimer >= frameDuration) {
        frameTimer -= frameDuration;
        currentFrame++;
        if (currentFrame < sheetCols * sheetRows) {
            int row = currentFrame / sheetCols;
            int col = currentFrame % sheetCols;
            sprite.setTextureRect(sf::IntRect(col * frameWidth, row * frameHeight, frameWidth, frameHeight));
        }
    }

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
    return currentFrame >= sheetCols * sheetRows;
}
