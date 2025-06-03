#include "Vehicle.h"
#include "ResourceManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <cmath>

using json = nlohmann::json;

Vehicle::Vehicle() {
    sprite.setTexture(ResourceManager::getInstance().getTexture("car"));
    position = { 300.f, 300.f };
    sprite.setPosition(position);
}

void Vehicle::update(float dt) {
    sf::Vector2f dir = directionVec;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0) dir /= len;

    position += dir * speed * dt;
    sprite.setPosition(position);

    float angle = std::atan2(dir.y, dir.x) * 180.f / 3.14159f + 90.f;
    sprite.setRotation(angle);
}



void Vehicle::draw(sf::RenderTarget& target) {
    target.draw(sprite);
}


sf::Vector2f Vehicle::getPosition() const {
    return position;
}

void Vehicle::setPosition(const sf::Vector2f& pos) {
    position = pos;
    sprite.setPosition(pos);
}

void Vehicle::move(const sf::Vector2f& direction, float dt) {
    position += direction * speed * dt;
}

float Vehicle::getSpeed() const {
    return speed;
}

void Vehicle::startTurn(sf::Vector2f from, sf::Vector2f control, sf::Vector2f to) {
    bezierP0 = from;
    bezierP1 = control;
    bezierP2 = to;
    bezierT = 0.f;
    inTurn = true;
    position = from;
    sprite.setPosition(position);
}

json loadRoadsFromFile(const std::string& filename) {
    std::ifstream in(filename);
    json data;
    in >> data;
    return data;
}

void Vehicle::setTexture(const sf::Texture& texture) {
    sprite.setTexture(texture);
}

void Vehicle::setDirectionVec(const std::string& dir) {
    if (dir == "up") directionVec = { 0.f, -1.f };
    else if (dir == "down") directionVec = { 0.f, 1.f };
    else if (dir == "left") directionVec = { -1.f, 0.f };
    else if (dir == "right") directionVec = { 1.f, 0.f };
    else directionVec = { 0.f, 0.f };  // ????? ????
}

void Vehicle::setScale(float scaleX, float scaleY) {
    sprite.setScale(scaleX, scaleY);
}
