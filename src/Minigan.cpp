#include "Minigan.h"
#include "Player.h"

Minigan::Minigan(const sf::Texture& texture, const sf::Vector2f& pos)
    : WeaponPresent(texture, pos) {
}

std::string Minigan::getType() const {
    return "Minigan";
}
