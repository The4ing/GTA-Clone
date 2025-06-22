
#include "SpeedBoost.h"
#include "Player.h"

SpeedBoost::SpeedBoost(const sf::Texture& texture, const sf::Vector2f& pos)
    : Present(texture, pos) {
}

std::string SpeedBoost::getType() const {
    return "SpeedBoost";
}

void SpeedBoost::applyEffect(Player& player) {
    player.increaseSpeed();
    collect();
}

