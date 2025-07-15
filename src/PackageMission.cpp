#include "PackageMission.h"
#include "Player.h"

PackageMission::PackageMission(const std::string& description, const sf::Vector2f& destination)
    : Mission(description), destination(destination), delivered(false) {
}

void PackageMission::start() {
    state = MissionState::InProgress;
}

void PackageMission::update(float, Player& player) {
    if (state != MissionState::InProgress) {
        return;
    }

    sf::Vector2f pos = player.getPosition();
    float dx = pos.x - destination.x;
    float dy = pos.y - destination.y;
    bool inArea = dx * dx + dy * dy < 32.f * 32.f;

    if (!delivered && inArea && player.getWantedLevel() == 0) {
        player.playThrowAnimation();
        player.getInventory().useItem("Package");
        delivered = true;
        state = MissionState::Completed;
    }
}

bool PackageMission::isCompleted() const {
    return state == MissionState::Completed;
}