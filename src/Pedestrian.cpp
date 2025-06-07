#include "Pedestrian.h"
#include "ResourceManager.h"
#include <cstdlib>
#include <cmath>

Pedestrian::Pedestrian() {
    sprite.setTexture(ResourceManager::getInstance().getTexture("pedestrian"));

    spriteRect = { 0, 0, frameWidth, frameHeight };
    sprite.setTextureRect(spriteRect);
    sprite.setScale(0.8f, 0.8f); // scaled down to match game size

    position = {
        static_cast<float>(rand() % 800),
        static_cast<float>(rand() % 600)
    };
    sprite.setPosition(position);

    setRandomDirection();
}

void Pedestrian::update(float dt) {
    timeSinceLastDirectionChange += dt;

    if (timeSinceLastDirectionChange >= directionChangeInterval) {
        timeSinceLastDirectionChange = 0.f;
        setRandomDirection();
    }

    sf::FloatRect bounds = sprite.getGlobalBounds();

    if (position.x < 0 || position.x + bounds.width > 800)
        direction.x = -direction.x;
    if (position.y < 0 || position.y + bounds.height > 600)
        direction.y = -direction.y;

    move(direction, dt);
    sprite.setPosition(position);

    // אנימציית הליכה
    animationTimer += dt;
    if (animationTimer >= animationSpeed) {
        animationTimer = 0.f;
        currentFrame = (currentFrame + 1) % maxWalkFrames;

        spriteRect.left = currentFrame * frameWidth;
        spriteRect.top = currentRow * frameHeight;
        sprite.setTextureRect(spriteRect);
    }
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

void Pedestrian::setRandomDirection() {
    do {
        direction = {
            static_cast<float>((rand() % 3) - 1),
            static_cast<float>((rand() % 3) - 1)
        };
    } while (direction == sf::Vector2f{ 0.f, 0.f });

    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length != 0)
        direction /= length;

    updateSpriteDirection();
}

void Pedestrian::updateSpriteDirection() {
   

    spriteRect.left = currentFrame * frameWidth;
    spriteRect.top = currentRow * frameHeight;
    sprite.setTextureRect(spriteRect);
}




void Pedestrian::die() {
    isDead = true;
    currentFrame = 0;
    currentRow = 3; // שורה שלישית במידת הצורך
    animationTimer = 0.f;
}

