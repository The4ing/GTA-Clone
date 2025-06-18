#include "HealthPresent.h"
#include "Player.h"

HealthPresent::HealthPresent(const sf::Texture& texture, const sf::Vector2f& pos)
    : Present(texture, pos) {
}

std::string HealthPresent::getType() const {
    return "Health";
}

void HealthPresent::applyEffect(Player& player) {
    player.heal(25);
    collect();
}
