#pragma once
#include "WeaponPresent.h"

class Knife : public WeaponPresent {
public:
    Knife(const sf::Texture& texture, const sf::Vector2f& pos);
    std::string getType() const override;

};