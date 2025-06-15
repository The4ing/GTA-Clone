#pragma once
#include "Present.h"

class WeaponPresent : public Present {
public:
    WeaponPresent(const sf::Texture& texture, const sf::Vector2f& pos);
    std::string getType() const override;
    void applyEffect(class Player& player) override;
};
