#include "Money.h"

Money::Money(const sf::Texture& texture, const sf::Vector2f& pos)
    : Present(texture, pos) {
}

std::string Money::getType() const {
    return "Money";
}

void Money::applyEffect(Player& player) {
    player.setMoney(20);
    collect();
}


