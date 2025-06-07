#include "Pedestrian.h"
#include "ResourceManager.h"
#include <cstdlib>
#include <cmath>

Pedestrian::Pedestrian() {
    sprite.setTexture(ResourceManager::getInstance().getTexture("pedestrian"));

    spriteRect = { 0, 0, frameWidth, frameHeight };
    sprite.setTextureRect(spriteRect);
    sprite.setScale(0.07f, 0.07f); // scaled down to match game size

    position = {
        static_cast<float>(rand() % 800),
        static_cast<float>(rand() % 600)
    };
    sprite.setPosition(position);

    setRandomDirection();
}

void Pedestrian::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    timeSinceLastDirectionChange += dt;

    if (timeSinceLastDirectionChange >= directionChangeInterval) {
        timeSinceLastDirectionChange = 0.f;
        setRandomDirection();
    }

    sf::Vector2f nextPos = position + direction * speed * dt;
    float radius = getCollisionRadius(); // ניצור פונקציה דומה לזו של Player

    bool collision = false;
    for (const auto& poly : blockedPolygons) {
        if (circleIntersectsPolygon(nextPos, radius, poly)) {
            collision = true;
            break;
        }
    }

    if (!collision) {
        position = nextPos;
        sprite.setPosition(position);
    }
    else {
        setRandomDirection(); // החלף כיוון אם נתקע
    }

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

float Pedestrian::getCollisionRadius() const {
    return 6.f; // כמו ב-Player או בהתאם לגודל הדמות
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



