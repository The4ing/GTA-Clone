#include "Player.h"
#include <SFML/Window/Keyboard.hpp>
#include "ResourceManager.h"
#include <cmath>

Player::Player()
    : frameWidth(0)
    , frameHeight(0)
    , currentFrame(0)
    , sheetCols(4)
    , sheetRows(2)
    , animTimer(0.f)
    , animDelay(0.1f)
{
    sf::Texture& texture = ResourceManager::getInstance().getTexture("player");
    sprite.setTexture(texture);

    int sheetW = texture.getSize().x;
    int sheetH = texture.getSize().y;
    frameWidth = sheetW / sheetCols;
    frameHeight = sheetH / sheetRows;

    sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
    sprite.setScale(0.25f, 0.25f);
    sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));

    position = { 100.f, 100.f };
    sprite.setPosition(position);
}

void Player::setPosition(const sf::Vector2f& pos) {
   
        sprite.setPosition(pos);
}

sf::Vector2f Player::getPosition() const {
    return sprite.getPosition();
}

void Player::update(float dt, const std::vector<sf::FloatRect>& blockedAreas) {
    sf::Vector2f movement(0.f, 0.f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        movement.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        movement.x += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        movement.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        movement.y += 1.f;

    bool isMoving = (movement.x != 0.f || movement.y != 0.f);

    if (isMoving) {
        
            float len = std::sqrt(movement.x * movement.x + movement.y * movement.y);
        if (len != 0.f)
            movement /= len;

    float angle = std::atan2(movement.y, movement.x) * 180.f / 3.14159f + 90.f;
        sprite.setRotation(angle);

    sf::FloatRect nextBounds = sprite.getGlobalBounds();
    nextBounds.left += movement.x * speed * dt;
    nextBounds.top += movement.y * speed * dt;

    bool collision = false;
    for (const auto& rect : blockedAreas) {
        if (nextBounds.intersects(rect)) {
            collision = true;
            break;
        }
    }

    if (!collision) {
        sprite.move(movement * speed * dt);
    }

    animTimer += dt;
    if (animTimer >= animDelay) {
        animTimer -= animDelay;
        const int walkingFrames = 4;
        currentFrame = (currentFrame + 1) % walkingFrames;
    }

    int col = currentFrame;
    int row = 0;
    int left = col * frameWidth;
    int top = row * frameHeight;
    sprite.setTextureRect(sf::IntRect(left, top, frameWidth, frameHeight));
    }
    else {
        animTimer = 0.f;
        currentFrame = 0;
        int left = 0 * frameWidth;
        int top = 1 * frameHeight;
        sprite.setTextureRect(sf::IntRect(left, top, frameWidth, frameHeight));
    }

}

void Player::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}