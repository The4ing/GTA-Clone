#include "Police.h"
#include "ResourceManager.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include "CollisionUtils.h"

const sf::FloatRect MAP_BOUNDS(0.f, 0.f, 4640.f, 4670.f);

Police::Police(sf::Vector2f target) :targetPos(target) {
    sprite.setTexture(ResourceManager::getInstance().getTexture("police"));
    framesPerRow = 6;
    frameWidth = sprite.getTexture()->getSize().x / framesPerRow;
    frameHeight = sprite.getTexture()->getSize().y;
    sprite.setTextureRect({ 0, 0, frameWidth, frameHeight });
    sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
    //sprite.setPosition(target);
    sprite.setPosition(100, 100);
    setRandomWanderDestination(MAP_BOUNDS);

     sprite.setScale(0.07f, 0.07f); 
    speed = 40.f;
    currentFrame = 0;
}

void Police::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    float dist = std::hypot(targetPos.x - getPosition().x, targetPos.y - getPosition().y);

    debugPrintTimer += dt;
    if (debugPrintTimer >= 2.f) { // הדפסה כל 2 שניות
        float dist = std::hypot(wanderDestination.x - getPosition().x, wanderDestination.y - getPosition().y);
        debugPrintTimer = 0.f;
    }


    if (dist <= detectionRadius) {
        if (state != PoliceState::Chasing) {
            state = PoliceState::Chasing;
            currentFrame = 0;
        }
    }
    else if (state == PoliceState::Chasing && dist > detectionRadius + 30.f) {
        state = PoliceState::Idle;
        currentFrame = 0;
    }

    if (state == PoliceState::Chasing) {
        moveToward(targetPos, dt, blockedPolygons);
    }
    else if (state == PoliceState::Idle) {
        float distToDest = std::hypot(wanderDestination.x - getPosition().x, wanderDestination.y - getPosition().y);
        if (distToDest < 10.f) { // Arrived
            setRandomWanderDestination(MAP_BOUNDS);
        }
        moveToward(wanderDestination, dt, blockedPolygons);
    }

    //else if (state == PoliceState::Idle) {
    //    wanderTimer -= dt;
    //    if (wanderTimer <= 0.f) {
    //        targetPos = getPosition() + sf::Vector2f(rand() % 100 - 50, rand() % 100 - 50);
    //        wanderTimer = 2.f + static_cast<float>(rand() % 300) / 100.f; 
    //    }
    //    moveToward(targetPos, dt, blockedPolygons);
    //}

    animationTimer += dt;
    if (animationTimer >= animationSpeed) {
        animationTimer = 0.f;
        if (std::abs(sprite.getRotation()) > 0.1f && (std::abs(targetPos.x - getPosition().x) > 1.f || std::abs(targetPos.y - getPosition().y) > 1.f)) {
            currentFrame = (currentFrame + 1) % framesPerRow;
        }
        else {
            currentFrame = 0;
        }
    }
    sprite.setTextureRect({
        currentFrame * frameWidth,
        0,
        frameWidth,
        frameHeight
        });
}

void Police::moveToward(const sf::Vector2f& target, float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    sf::Vector2f dir = target - getPosition();
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.f)
        dir /= len;
    else
        dir = sf::Vector2f(0.f, 1.f);


    float angle = std::atan2(dir.y, dir.x) * 180.f / M_PI;
    sprite.setRotation(angle - 270.f);

    sf::Vector2f nextPos = sprite.getPosition() + dir * speed * dt;
    bool collision = false;
    for (const auto& poly : blockedPolygons) {
        if (circleIntersectsPolygon(nextPos, getCollisionRadius(), poly)) {
            collision = true;
            break;
        }
    }

    if (!collision) {
        sprite.move(dir * speed * dt);
    }
}

void Police::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

void Police::takeDamage(int amount) {
    health -= amount;
    if (health < 0) health = 0;
}

bool Police::isDead() const {
    return health <= 0;
}

float Police::getCollisionRadius() const {
    return 12.f;
}

void Police::setTargetPosition(const sf::Vector2f& pos) {
    targetPos = pos;
}

void Police::setRandomWanderDestination(const sf::FloatRect& mapBounds) {
    sf::Vector2f candidate;
    int tries = 0;
    bool found = false;

    do {
        float angle = static_cast<float>(rand()) / RAND_MAX * 2 * M_PI;
        float radius = static_cast<float>(rand()) / RAND_MAX * 500.f;
        candidate = getPosition() + sf::Vector2f(std::cos(angle), std::sin(angle)) * radius;
        if (mapBounds.contains(candidate))
            found = true;
        ++tries;
    } while (!found && tries < 20);

    wanderDestination = candidate;
}
