#include "Police.h"
#include "ResourceManager.h"
#include <cmath>
#include <algorithm>

Police::Police(sf::Vector2f target) :targetPos(target){
    sprite.setTexture(ResourceManager::getInstance().getTexture("police"));
    frameWidth = sprite.getTexture()->getSize().x / 4;
    frameHeight = sprite.getTexture()->getSize().y / 2;
    sprite.setTextureRect({ 0, 0, frameWidth, frameHeight });
    sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
    sprite.setScale(0.25f, 0.25f);
    speed = 40.f;
}

void Police::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    float dist = std::hypot(targetPos.x - getPosition().x, targetPos.y - getPosition().y);

    if (dist <= detectionRadius) {
        state = PoliceState::Chasing;
    }
    else if (state == PoliceState::Chasing && dist > detectionRadius + 30.f) {
        state = PoliceState::Idle; // יצא מטווח, חוזר לשיטוט
    }

    if (state == PoliceState::Chasing) {
        moveToward(targetPos, dt, blockedPolygons);
    }
    else if (state == PoliceState::Idle) {
        wanderTimer -= dt;
        if (wanderTimer <= 0.f) {
            
            targetPos = getPosition() + sf::Vector2f(rand() % 100 - 50, rand() % 100 - 50);
            wanderTimer = 2.f + static_cast<float>(rand() % 300) / 100.f; // בין 2 ל-5 שניות
        }

        moveToward(targetPos, dt, blockedPolygons);
    }
}


void Police::moveToward(const sf::Vector2f& target, float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    sf::Vector2f dir = target - getPosition();
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.f)
        dir /= len;

    float angle = std::atan2(dir.y, dir.x) * 180.f / 3.14159f + 90.f;
    sprite.setRotation(angle);

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
    return 6.f; // כמו ב-Player או בהתאם לגודל הדמות
}

void Police::setTargetPosition(const sf::Vector2f& pos) {
    targetPos = pos;
}
