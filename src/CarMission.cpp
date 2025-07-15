#include "CarMission.h"
#include "Vehicle.h"


CarMission::CarMission(const std::string& description)
    : Mission(description), inCar(false), timeInCar(0.f), timeAtSpeed(0.f) {
}

void CarMission::start() {
    state = MissionState::InProgress;
    // Reset mission specific timers
    inCar = false;
    timeInCar = 0.f;
    timeAtSpeed = 0.f;
}

void CarMission::update(float dt, Player& player) {
    if (state != MissionState::InProgress) {
        return;
    }

    if (player.isInVehicle()) {
        if (!inCar) {
            inCar = true;
        }

        timeInCar += dt;

        Vehicle* vehicle = player.getCurrentVehicle();
        if (vehicle && vehicle->getSpeed() >= 100.f) {
            timeAtSpeed += dt;
        }
        else {
            timeAtSpeed = 0.f;
        }

        if (timeAtSpeed >= 7.f && player.getWantedLevel() == 0) {
            state = MissionState::Completed;
        }
    }
    else {
        inCar = false;
        timeInCar = 0.f;
        timeAtSpeed = 0.f;
    }
}

bool CarMission::isCompleted() const {
    return state == MissionState::Completed;
}
