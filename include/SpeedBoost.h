#pragma once
#include "Present.h"

class SpeedBoost : public Present {
public:
    SpeedBoost(const sf::Texture& texture, const sf::Vector2f& pos);
    std::string getType() const override;
    void applyEffect(class Player& player) override;
    int getPrice() const override;

};
