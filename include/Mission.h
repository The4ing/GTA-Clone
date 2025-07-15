#pragma once
#include <SFML/Graphics.hpp>
#include <string>

enum class MissionState { NotStarted, InProgress, Completed };

class Mission {
public:
    Mission(const std::string& description);
    virtual ~Mission() = default;

    virtual void start() = 0;
    virtual void update(float dt, class Player& player) = 0;
    virtual bool isCompleted() const = 0;

    const std::string& getDescription() const;
    MissionState getState() const;

protected:
    std::string description;
    MissionState state;
};
