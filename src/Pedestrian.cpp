#include "Pedestrian.h"
#include "ResourceManager.h"
#include <cstdlib>
#include <cmath>
#include "Constants.h"
#include "CollisionUtils.h"
#include "SoundManager.h"

Pedestrian::Pedestrian(sf::Vector2f pos) : position(pos) {
    sprite.setTexture(ResourceManager::getInstance().getTexture("pedestrian"));
    characterRow = rand() % numCharacters;
    currentFrame = 0;
    sprite.setScale(0.5f, 0.5f);
    sprite.setPosition(position);

    setRandomDirection();
    nextDirection = direction;
    updateSprite();
}

void Pedestrian::startBackingUp() {
    isBackingUp = true;
    backupProgress = 0.f;
}

bool Pedestrian::getIsBackingUp() const {
    return isBackingUp;
}

void Pedestrian::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {

    if (dying) {
        deathTimer += dt;
        if (deathTimer >= deathDuration) {
            remove = true;
        }
        return;
    }

    float stepSize = speed * dt;

    // ????? ?????
    if (isBackingUp) {
        float remaining = backupDistance - backupProgress;
        float moveStep = std::min(stepSize, remaining);
        sf::Vector2f backDir = -direction;

        sf::Vector2f nextPos = position + backDir * moveStep;

        if (checkCollision(position, nextPos, blockedPolygons, getCollisionRadius())) {
            isBackingUp = false;
            backupProgress = 0.f;
            setRandomDirection();
            nextDirection = direction;
        }
        else {
            position = nextPos;
            sprite.setPosition(position);
            backupProgress += moveStep;
            if (backupProgress >= backupDistance) {
                isBackingUp = false;
                backupProgress = 0.f;
                setRandomDirection();
                nextDirection = direction;
            }
        }
        updateSprite();
        return;
    }

    // ????? ??????
    if (isIdle) {
        idleTimer -= dt;
        if (idleTimer <= 0.f) {
            isIdle = false;
            setRandomDirection();
            nextDirection = direction;
        }
        return;
    }

    timeSinceLastDirectionChange += dt;
    if (timeSinceLastDirectionChange >= directionChangeInterval) {
        timeSinceLastDirectionChange = 0.f;

        if ((rand() % 1000) / 1000.f < idleProbability) {
            isIdle = true;
            idleTimer = idleDurationMin + static_cast<float>(rand()) / RAND_MAX * (idleDurationMax - idleDurationMin);
            direction = { 0.f, 0.f };
            updateSprite();
            return;
        }

        // ??? ????? ??? ??? (nextDirection)
        sf::Vector2f newDir;
        do {
            newDir = {
                static_cast<float>((rand() % 201 - 100) / 100.f),
                static_cast<float>((rand() % 201 - 100) / 100.f)
            };
            float len = std::sqrt(newDir.x * newDir.x + newDir.y * newDir.y);
            if (len > 0.001f)
                newDir /= len;
        } while (newDir == sf::Vector2f{ 0.f, 0.f });

        nextDirection = newDir;
    }

    // ????? ?????? ?? ?????
    float turnSpeed = 3.f;
    direction += (nextDirection - direction) * turnSpeed * dt;

    float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (len > 0.001f)
        direction /= len;
    else
        setRandomDirection();

    sf::Vector2f nextPos = position + direction * stepSize;

    if (checkCollision(position, nextPos, blockedPolygons, getCollisionRadius())) {
        startBackingUp();
    }
    else {
        position = nextPos;
        sprite.setPosition(position);
    }

    // ????? ??????? ??????
    animationTimer += dt;
    if (animationTimer >= animationSpeed) {
        animationTimer = 0.f;
        if (std::abs(direction.x) > 0.01f || std::abs(direction.y) > 0.01f) {
            currentFrame = (currentFrame == 1) ? 2 : 1;
        }
        else {
            currentFrame = 0;
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

bool Pedestrian::checkCollision(const sf::Vector2f& currentPos, const sf::Vector2f& nextPos, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons, float radius) {
    const int steps = 5;
    for (int i = 1; i <= steps; ++i) {
        float t = i / float(steps);
        sf::Vector2f interpPos = currentPos + (nextPos - currentPos) * t;

        if (CollisionUtils::isInsideBlockedPolygon(interpPos, blockedPolygons)) {
            return true;
        }
    }
    return false;
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
    return 10.f;
}

void Pedestrian::setRandomDirection() {
    sf::Vector2f newDir;
    do {
        newDir = {
            static_cast<float>((rand() % 3) - 1),
            static_cast<float>((rand() % 3) - 1)
        };
    } while (newDir == sf::Vector2f{ 0.f, 0.f });

    direction = newDir;
    float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (len > 0.001f) direction /= len;

    updateSprite();
}

void Pedestrian::updateSprite() {
    int left = currentFrame * frameWidth;
    int top = characterRow * frameHeight;
    sprite.setTextureRect(sf::IntRect(left, top, frameWidth, frameHeight));
    sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
    sprite.setPosition(position);
}

sf::FloatRect Pedestrian::getCollisionBounds() const {
    float radius = getCollisionRadius();
    return sf::FloatRect(
        position.x - radius,
        position.y - radius,
        radius * 2.f,
        radius * 2.f
    );
}

void Pedestrian::takeDamage(float amount) {
    if (dying)
        return;
    health -= static_cast<int>(amount);
    if (health < 0)
        health = 0;

    static const std::vector<std::string> hurtSounds = {
        "hurt1", "hurt2", "hurt3", "hurt4"
    };
    SoundManager::getInstance().playRandomSound(hurtSounds, 0.95f, 1.05f);


    if (health == 0) {
        dying = true;
        deathTimer = 0.f;
    }
}

bool Pedestrian::isDead() const {
    return health <= 0;
}