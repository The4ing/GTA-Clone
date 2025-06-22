#pragma once
#include "Present.h"

class PistolPresent : public Present {
public:
    PistolPresent(const sf::Texture& texture, const sf::Vector2f& pos);
    std::string getType() const override;
    void applyEffect(class Player& player) override;
};
