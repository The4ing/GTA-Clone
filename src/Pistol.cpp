#include "Pistol.h"





Pistol::Pistol(const sf::Texture& texture, const sf::Vector2f& pos)
    : WeaponPresent(texture, pos) {
}

std::string Pistol::getType() const {
    return "Pistol";
}



