#pragma once
#include "Character.h"

class Player : public Character {
public:
    Player();
    void update(float dt) override;
};
