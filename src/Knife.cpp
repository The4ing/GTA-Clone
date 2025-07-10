#include "Knife.h"
#include "Player.h"

Knife::Knife(const sf::Texture& texture, const sf::Vector2f& pos)
    : WeaponPresent(texture, pos) {
}

std::string Knife::getType() const {
    return "Knife";
}



