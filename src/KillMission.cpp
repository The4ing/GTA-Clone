#include "KillMission.h"
#include "Player.h"

KillMission::KillMission(const std::string& description, KillTarget target, int count)
    : Mission(description), target(target), requiredKills(count), waitingForWanted(false) {
}

void KillMission::start() {
    state = MissionState::InProgress;
    waitingForWanted = false;
}

void KillMission::update(float dt, Player& player) {
    if (state != MissionState::InProgress)
        return;

    int kills = (target == KillTarget::NPC) ? player.getNpcKills() : player.getCopKills();
    if (!waitingForWanted && kills >= requiredKills) {
        waitingForWanted = true;
    }

    if (waitingForWanted && player.getWantedLevel() == 0) {
        state = MissionState::Completed;
    }
}

bool KillMission::isCompleted() const {
    return state == MissionState::Completed;
}