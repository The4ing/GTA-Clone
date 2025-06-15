#pragma once
#include "Present.h"

class AmmoPresent : public Present {
public:
    AmmoPresent(const sf::Texture& texture, const sf::Vector2f& pos);
    std::string getType() const override;
    void applyEffect(class Player& player) override;
};
