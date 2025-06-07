#include "PoliceCar.h"
#include "ResourceManager.h"
#include <cmath>

PoliceCar::PoliceCar() {
    sprite.setTexture(ResourceManager::getInstance().getTexture("police_car"));
    frameWidth = sprite.getTexture()->getSize().x;
    frameHeight = sprite.getTexture()->getSize().y;
    sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
    sprite.setScale(0.2f, 0.2f);
}

void PoliceCar::update(float dt, const std::vector<sf::Vector2f>& patrolPath) {
    if (patrolPath.empty()) return;

    const sf::Vector2f& target = patrolPath[currentPoint];
    sf::Vector2f dir = target - sprite.getPosition();
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    if (len < 5.f) {
        currentPoint = (currentPoint + 1) % patrolPath.size();
        return;
    }

    dir /= len;

    float angle = std::atan2(dir.y, dir.x) * 180.f / 3.14159f + 90.f;
    sprite.setRotation(angle);
    sprite.move(dir * speed * dt);
}

void PoliceCar::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

void PoliceCar::setTarget(const sf::Vector2f& target) {
    // Optional: move directly toward a specific location
}
