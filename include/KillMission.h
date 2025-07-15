#pragma once
#include "Mission.h"

enum class KillTarget { NPC, Cop };

class KillMission : public Mission {
public:
    KillMission(const std::string& description, KillTarget target, int count);

    void start() override;
    void update(float dt, Player& player) override;
    bool isCompleted() const override;

private:
    KillTarget target;
    int requiredKills;
    bool waitingForWanted;
};
