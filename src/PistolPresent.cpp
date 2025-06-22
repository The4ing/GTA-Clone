#include "PistolPresent.h"
#include "Player.h"

PistolPresent::PistolPresent(const sf::Texture& texture, const sf::Vector2f& pos)
    : Present(texture, pos) {
}

std::string PistolPresent::getType() const {
    return "Weapon";
}

void PistolPresent::applyEffect(Player& player) {
   // player.giveWeapon();
    collect();
}
