#include "Minigun.h"
#include "Player.h"

Minigun::Minigun(const sf::Texture& texture, const sf::Vector2f& pos)
    : WeaponPresent(texture, pos) {
}

std::string Minigun::getType() const {
    return "Minigun";
}

int Minigun::getPrice() const
{
    return 0;
}
