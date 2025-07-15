#include "Explosion.h"
#include "ResourceManager.h"
#include <iostream>

Explosion::Explosion(const sf::Vector2f& pos, float radius)
    : position(pos), timer(0.f), frameTimer(0.f), frameDuration(0.05f),
    currentFrame(0), frameWidth(0), frameHeight(0), duration(1.f),
    m_radius(1.f)
{
    if (radius <= 0.f) {
        std::cerr << "Warning: Explosion created with non-positive radius "
            << radius << ". Using radius = 1" << std::endl;
        m_radius = 1.f;
    }
    else {
        m_radius = radius;
    }
    sprite.setTexture(ResourceManager::getInstance().getTexture("explosion"));
    sf::Vector2u texSize = sprite.getTexture()->getSize();
    frameWidth = texSize.x / sheetCols;
    frameHeight = texSize.y / sheetRows;
    sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
    sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));

    float scaleX = (m_radius * 2.f) / static_cast<float>(frameWidth);
    float scaleY = (m_radius * 2.f) / static_cast<float>(frameHeight);
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
