#include "AmmoPresent.h"
#include "Player.h"

AmmoPresent::AmmoPresent(const sf::Texture& texture, const sf::Vector2f& pos)
    : Present(texture, pos) {
}

std::string AmmoPresent::getType() const {
    return "Ammo";
}

void AmmoPresent::applyEffect(Player& player) {
    //player.addAmmo(30);
    collect();
}
