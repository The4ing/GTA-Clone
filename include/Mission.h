#pragma once
#include <SFML/Graphics.hpp>
#include <string>

enum class MissionState { NotStarted, InProgress, Completed };

class Mission {
public:
    Mission();
    void setDestination(const sf::Vector2f& dest);
    void setDescription(const std::string& desc);
    const std::string& getDescription() const;
    const sf::Vector2f& getDestination() const;
    void start();
    MissionState getState() const;
    void update(float dt, class Player& player);

private:
    MissionState state;
    sf::Vector2f destination;
    std::string description;
    bool delivered;
};