#pragma once
#include "Present.h"

class WeaponPresent : public Present {
public:
    WeaponPresent(const sf::Texture& texture, const sf::Vector2f& pos);
   
   


    void applyEffect(Player& player) override;

    virtual std::string getType() const = 0; // פונקציה וירטואלית לכל נשק
};
