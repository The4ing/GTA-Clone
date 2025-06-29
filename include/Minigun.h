#pragma once
#include "WeaponPresent.h"

class Minigun : public WeaponPresent {
public:
    Minigun(const sf::Texture& texture, const sf::Vector2f& pos);
    std::string getType() const override;
     int getPrice() const;

};