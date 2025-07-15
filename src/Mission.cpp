#include "Mission.h"

Mission::Mission(const std::string& description)
    : description(description), state(MissionState::NotStarted) {
}

const std::string& Mission::getDescription() const {
    return description;
}

MissionState Mission::getState() const {
    return state;
}
