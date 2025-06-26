#pragma once
#include "WeaponPresent.h"

class Minigan : public WeaponPresent {
public:
    Minigan(const sf::Texture& texture, const sf::Vector2f& pos);
    std::string getType() const override;

};