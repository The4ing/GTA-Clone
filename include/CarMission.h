#pragma once
#include "Mission.h"
#include "Player.h"

class CarMission : public Mission {
public:
    CarMission(const std::string& description);

    void start() override;
    void update(float dt, Player& player) override;
    bool isCompleted() const override;

private:
    bool inCar;
    float timeInCar;
    float timeAtSpeed;
};
