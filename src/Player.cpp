#include "Player.h"
#include <SFML/Window/Keyboard.hpp>
#include "ResourceManager.h"

Player::Player() {
    sprite.setTexture(ResourceManager::getInstance().getTexture("player"));
    position = { 100.f, 100.f };
    sprite.setPosition(position);
}

void Player::update(float dt) {
    sf::Vector2f dir(0.f, 0.f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) dir.y -= 1;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) dir.y += 1;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) dir.x -= 1;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) dir.x += 1;

    if (dir != sf::Vector2f(0.f, 0.f)) {
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        dir /= length; 
        move(dir, dt);

    }

    Character::update(dt);
}
