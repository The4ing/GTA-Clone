#include "Pistol.h"
#include "Player.h"

Pistol::Pistol(const sf::Texture& texture, const sf::Vector2f& pos)
    : WeaponPresent(texture, pos) {
}

std::string Pistol::getType() const {
    return "Pistol";
}

int Pistol::getPrice() const
{
    return 0;
}

