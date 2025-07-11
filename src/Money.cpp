#include "Money.h"

Money::Money(const sf::Texture& texture, const sf::Vector2f& pos)
    : Present(texture, pos), tempMoney(false){
}

std::string Money::getType() const {
    return "Money";
}

bool Money::isExpired() const
{
    return m_clock.getElapsedTime().asSeconds() >= MAX_LIFETIME;
}

void Money::applyEffect(Player& player) {
    player.setMoney(20);
    collect();
}

bool Money::getTempMoney() const
{
    return tempMoney;
}

void Money::setTempMoney(bool updt)
{
    tempMoney = updt;
}





