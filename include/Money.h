#pragma once
#include "Present.h"
#include "Player.h"

class Money : public Present {
public:
    Money(const sf::Texture& texture, const sf::Vector2f& pos);
    std::string getType() const override;
    void applyEffect(class Player& player) override;


};
