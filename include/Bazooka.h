#pragma once
#include "WeaponPresent.h"

class Bazooka : public WeaponPresent {
public:
    Bazooka(const sf::Texture& texture, const sf::Vector2f& pos);
    std::string getType() const override;
    
};
