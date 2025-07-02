#pragma once
#include <SFML/System/Vector2.hpp>
#include <vector>

// Forward declarations
class Police;
class PoliceCar;

struct PatrolZone {
    int id;
    sf::Vector2f center;
    float radius;
    std::vector<Police*> assignedFootPatrols;
    PoliceCar* assignedPatrolCar;

    PatrolZone(int id, sf::Vector2f c, float r) : id(id), center(c), radius(r), assignedPatrolCar(nullptr) {}
};