#pragma once
#include "WeaponPresent.h"


class Grenade : public WeaponPresent {
public:
    Grenade(const sf::Texture& texture, const sf::Vector2f& pos);
    std::string getType() const override;

};
