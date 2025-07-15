#include "SurviveMission.h"
#include "Player.h"

SurviveMission::SurviveMission(const std::string& description, float duration)
    : Mission(description), requiredTime(duration), timer(0.f), waitingForWanted(false) {
}

void SurviveMission::start() {
    state = MissionState::InProgress;
    timer = 0.f;
    waitingForWanted = false;
}

void SurviveMission::update(float dt, Player& player) {
    if (state != MissionState::InProgress)
        return;

    if (!waitingForWanted) {
        timer += dt;
        if (timer >= requiredTime) {
            player.setWantedLevel(1);
            waitingForWanted = true;
        }
    }
    else {
        if (player.getWantedLevel() == 0) {
            state = MissionState::Completed;
        }
    }
}

bool SurviveMission::isCompleted() const {
    return state == MissionState::Completed;
}
