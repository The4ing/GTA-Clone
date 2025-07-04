#pragma once
#include "Present.h"
#include "Player.h"

class HealthPresent : public Present {
public:
    HealthPresent(const sf::Texture& texture, const sf::Vector2f& pos);
    std::string getType() const override;
    void applyEffect(class Player& player) override;
    int getPrice() const override;

};
