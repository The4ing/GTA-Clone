#include "Rifle.h"
#include "Player.h"

Rifle::Rifle(const sf::Texture& texture, const sf::Vector2f& pos)
    : WeaponPresent(texture, pos) {
}

std::string Rifle::getType() const {
    return "Rifle";
}


