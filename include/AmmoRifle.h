#pragma once
#include "Present.h"

class AmmoRifle  {
public:
    AmmoRifle();
    std::string getType() const ;
    void applyEffect(class Player& player) ;
};
