#pragma once
#include "Character.h"

class NPC : public Character {
public:
    NPC();
    void update(float dt) override;
};
