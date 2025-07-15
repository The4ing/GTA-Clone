#pragma once
#include "Mission.h"

class SurviveMission : public Mission {
public:
    SurviveMission(const std::string& description, float duration);

    void start() override;
    void update(float dt, Player& player) override;
    bool isCompleted() const override;

private:
    float requiredTime;
    float timer;
    bool waitingForWanted;
};
