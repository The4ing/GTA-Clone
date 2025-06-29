#include "Grenade.h"
#include "Player.h"

Grenade::Grenade(const sf::Texture& texture, const sf::Vector2f& pos)
    : WeaponPresent(texture, pos) {
}

std::string Grenade::getType() const {
    return "Grenade";
}

int Grenade::getPrice() const
{
    return 0;
}
