#include "Bazooka.h"
#include "Player.h"

Bazooka::Bazooka(const sf::Texture& texture, const sf::Vector2f& pos)
    : WeaponPresent(texture, pos) {
}

std::string Bazooka::getType() const {
    return "Bazooka";
}