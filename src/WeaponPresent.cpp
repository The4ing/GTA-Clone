#include "WeaponPresent.h"
#include "Player.h"

WeaponPresent::WeaponPresent(const sf::Texture& texture, const sf::Vector2f& pos)
    : Present(texture, pos) {
}

std::string WeaponPresent::getType() const {
    return "Weapon";
}

void WeaponPresent::applyEffect(Player& player) {
   // player.giveWeapon();
    collect();
}
