#include "WeaponPresent.h"
#include "Player.h"

WeaponPresent::WeaponPresent(const sf::Texture& texture, const sf::Vector2f& pos)
    : Present(texture, pos) {
}


void WeaponPresent::applyEffect(Player& player)
{
    player.AddWeapon(getType());  
    collect();
}
