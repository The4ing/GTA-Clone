#pragma once
#include "Mission.h"
#include "Player.h"

class PackageMission : public Mission {
public:
    PackageMission(const std::string& description, const sf::Vector2f& destination);

    void start() override;
    void update(float dt, Player& player) override;
    bool isCompleted() const override;

private:
    sf::Vector2f destination;
    bool delivered;
};