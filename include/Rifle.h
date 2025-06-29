#pragma once
#include "WeaponPresent.h"

class Rifle : public WeaponPresent {
public:
    Rifle(const sf::Texture& texture, const sf::Vector2f& pos);
    std::string getType() const override;
     int getPrice() const;

};