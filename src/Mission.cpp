#include "Mission.h"
#include "player.h"

Mission::Mission() : state(MissionState::NotStarted), destination(0.f, 0.f), delivered(false) {}

void Mission::setDestination(const sf::Vector2f& dest) { destination = dest; }
void Mission::setDescription(const std::string& desc) { description = desc; }
const std::string& Mission::getDescription() const { return description; }
const sf::Vector2f& Mission::getDestination() const { return destination; }
void Mission::start() { state = MissionState::InProgress; }
MissionState Mission::getState() const { return state; }

void Mission::update(float, Player& player) {
    if (state != MissionState::InProgress) return;
    sf::Vector2f pos = player.getPosition();
    float dx = pos.x - destination.x;
    float dy = pos.y - destination.y;
    if (!delivered && dx * dx + dy * dy < 32.f * 32.f) {
        if (player.getInventory().getCount("Package") > 0) {
            delivered = true;
            // remove package from inventory
            player.getInventory().useItem("Package");
        }
    }
    if (delivered && player.getWantedLevel() == 0) {
        state = MissionState::Completed;
    }
}