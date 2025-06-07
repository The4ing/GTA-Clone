#include "Player.h"
#include <SFML/Window/Keyboard.hpp>
#include "ResourceManager.h"
#include <cmath>
#include <algorithm> // for std::clamp

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



void Player::takeDamage(int amount)
{
}






void Player::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    sf::Vector2f movement(0.f, 0.f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        movement.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        movement.x += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        movement.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        movement.y += 1.f;

    bool isMoving = (movement.x != 0.f || movement.y != 0.f);

    if (isMoving) {
        float len = std::sqrt(movement.x * movement.x + movement.y * movement.y);
        if (len != 0.f)
            movement /= len;

        float angle = std::atan2(movement.y, movement.x) * 180.f / 3.14159f + 90.f;
        sprite.setRotation(angle);

        sf::Vector2f nextPos = sprite.getPosition() + movement * speed * dt;
        float radius = getCollisionRadius();

        bool collision = false;
        for (const auto& poly : blockedPolygons) {
            if (circleIntersectsPolygon(nextPos, radius, poly)) {
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
    sf::CircleShape circle(getCollisionRadius());
    circle.setOrigin(getCollisionRadius(), getCollisionRadius());
    circle.setPosition(getCenter());
    circle.setFillColor(sf::Color::Transparent);
    circle.setOutlineColor(sf::Color::Blue);
    circle.setOutlineThickness(1.f);
    window.draw(circle);


    window.draw(sprite);
}

sf::FloatRect Player::getCollisionBounds(const sf::Vector2f& offset) const {
    // Get the center position of the sprite (already scaled)
    sf::Vector2f pos = sprite.getPosition();
    sf::Vector2f size(frameWidth * sprite.getScale().x, frameHeight * sprite.getScale().y);

    return {
        pos.x - size.x / 2.f + offset.x,
        pos.y - size.y / 2.f + offset.y,
        size.x,
        size.y
    };
}

sf::Vector2f Player::getCenter() const {
    sf::Vector2f pos = sprite.getPosition();
    return { pos.x - 2.f, pos.y + 4.f }; // 🔁 שמאלה ולמטה
}

float Player::getCollisionRadius() const {
    // Set to match visual size — adjust as needed
    return 6.f; // in pixels
}
