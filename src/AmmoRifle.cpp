#include "AmmoRifle.h"
#include "Player.h"

AmmoRifle::AmmoRifle()
    
{
}

std::string AmmoRifle::getType() const {
    return "Ammo";
}

void AmmoRifle::applyEffect(Player& player) {
    //player.addAmmo(30);
    //collect();
}
