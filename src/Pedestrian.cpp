#include "Pedestrian.h"
#include "ResourceManager.h"
#include <cstdlib>
#include <cmath>
#include "Constants.h"
#include "CollisionUtils.h"

Pedestrian::Pedestrian(sf::Vector2f pos) : position(pos) {
    sprite.setTexture(ResourceManager::getInstance().getTexture("pedestrian"));
    characterRow = rand() % numCharacters;
    currentFrame = 0;
    sprite.setScale(0.5f, 0.5f);

   
    sprite.setPosition(position);

    setRandomDirection();
    updateSprite();
}

void Pedestrian::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    timeSinceLastDirectionChange += dt;
    if (timeSinceLastDirectionChange >= directionChangeInterval) {
        timeSinceLastDirectionChange = 0.f;
        setRandomDirection();
    }

    sf::Vector2f nextPos = position + direction * speed * dt;
    float radius = getCollisionRadius();
    bool collision = false;
    for (const auto& poly : blockedPolygons) {
        if (CollisionUtils::pointInPolygon(nextPos, poly)) {
            collision = true;
            break;
        }
    }

    if (!collision) {
        position = nextPos;
        sprite.setPosition(position);
    }
    else {
        setRandomDirection();
    }

    // Animation
    animationTimer += dt;
    if (animationTimer >= animationSpeed) {
        animationTimer = 0.f;
        if (std::abs(direction.x) > 0.01f || std::abs(direction.y) > 0.01f) {
            currentFrame = (currentFrame == 1) ? 2 : 1; 
        }
        else {
            currentFrame = 0; // Stand
        }
        updateSprite();
    }

    if (std::abs(direction.x) > 0.01f || std::abs(direction.y) > 0.01f) {
        float angle = std::atan2(direction.y, direction.x) * 180.f / M_PI;
        sprite.setRotation(angle + 270.f); 
    }
    else {
        sprite.setRotation(90.f); 
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
float Pedestrian::getSpeed() const { return speed; }
float Pedestrian::getCollisionRadius() const { return 10.f; }

void Pedestrian::setRandomDirection() {
    do {
        direction = {
            static_cast<float>((rand() % 3) - 1),
            static_cast<float>((rand() % 3) - 1)
        };
    } while (direction == sf::Vector2f{ 0.f, 0.f });

    updateSprite();
}

void Pedestrian::updateSprite() {
    int left = currentFrame * frameWidth;
    int top = characterRow * frameHeight;
    sprite.setTextureRect(sf::IntRect(left, top, frameWidth, frameHeight));
    sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
    sprite.setPosition(position);
}
