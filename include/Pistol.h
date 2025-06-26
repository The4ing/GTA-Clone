#pragma once
#include "WeaponPresent.h"

class Pistol : public WeaponPresent {
public:
    Pistol(const sf::Texture& texture, const sf::Vector2f& pos);
    std::string getType() const override;
    
};