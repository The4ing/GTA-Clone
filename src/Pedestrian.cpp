#include "Pedestrian.h"
#include "ResourceManager.h"
#include <cstdlib>
#include <cmath>

Pedestrian::Pedestrian() {
    sprite.setTexture(ResourceManager::getInstance().getTexture("pedestrian"));

    position = {
        static_cast<float>(rand() % 800),
        static_cast<float>(rand() % 600)
    };
    sprite.setPosition(position);

    setRandomDirection();
}

void Pedestrian::update(float dt) {
    timeSinceLastDirectionChange += dt;

    // החלפת כיוון כל 2 שניות
    if (timeSinceLastDirectionChange >= directionChangeInterval) {
        timeSinceLastDirectionChange = 0.f;
        setRandomDirection();
    }

    // בדיקה אם יוצא מהמסך, ואז שינוי כיוון
    sf::FloatRect bounds = sprite.getGlobalBounds();

    if (position.x < 0 || position.x + bounds.width > 800)
        direction.x = -direction.x;
    if (position.y < 0 || position.y + bounds.height > 600)
        direction.y = -direction.y;

    move(direction, dt);
    sprite.setPosition(position);
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

// פונקציית עזר לקביעת כיוון אקראי ונורמלי
void Pedestrian::setRandomDirection() {
    do {
        direction = {
            static_cast<float>((rand() % 3) - 1),
            static_cast<float>((rand() % 3) - 1)
        };
    } while (direction == sf::Vector2f{ 0.f, 0.f });

    // נורמליזציה – מהירות אחידה לכל כיוון
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length != 0)
        direction /= length;
}
