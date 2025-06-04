#include "Vehicle.h"
#include "ResourceManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <cmath>
#include <iostream>

using json = nlohmann::json;

Vehicle::Vehicle() {
    sprite.setTexture(ResourceManager::getInstance().getTexture("car"));

    // ????? origin ????? ??????
    sf::FloatRect bounds = sprite.getLocalBounds();
    sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

    position = { 300.f, 300.f };
    sprite.setPosition(position);
}

void Vehicle::update(float dt) {
    if (inTurn) {
        // ????? ??? ????? ?????
        bezierT += dt * 1.f;  // ?????? ???????? ??????
        if (bezierT >= 1.f) {
            bezierT = 1.f;
            inTurn = false;  // ???? ?? ??????
        }

        // ????? ????? ???????
        float t = bezierT;
        float oneMinusT = 1.f - t;
        sf::Vector2f pos =
            oneMinusT * oneMinusT * bezierP0 +
            2.f * oneMinusT * t * bezierP1 +
            t * t * bezierP2;

        directionVec = bezierP2 - bezierP0;  // ????? ????
        float len = std::sqrt(directionVec.x * directionVec.x + directionVec.y * directionVec.y);
        if (len > 0) directionVec /= len;

        position = pos;
        sprite.setPosition(pos);

        float angle = std::atan2(directionVec.y, directionVec.x) * 180.f / 3.14159f + 90.f;
        sprite.setRotation(angle);
    }
    else {
        // ????? ????? ??? ???
        sf::Vector2f dir = directionVec;
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 0) dir /= len;

        position += dir * speed * dt;
        sprite.setPosition(position);

        float angle = std::atan2(dir.y, dir.x) * 180.f / 3.14159f + 90.f;
        sprite.setRotation(angle);
    }
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
    std::cout << "Turning from " << from.x << "," << from.y << " to " << to.x << "," << to.y << std::endl;

}


json loadRoadsFromFile(const std::string& filename) {
    std::ifstream in(filename);
    json data;
    in >> data;
    return data;
}

void Vehicle::setTexture(const sf::Texture& texture) {
    sprite.setTexture(texture);

    // ????? origin ?? ??? (????? ????????? ????)
    sf::FloatRect bounds = sprite.getLocalBounds();
    sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
}


void Vehicle::setDirectionVec(const std::string& dir) {
    currentDirectionStr = dir; // ????? ?? ???????

    if (dir == "up") {
        directionVec = { 0.f, -1.f };
        sprite.setRotation(0.f);
    }
    else if (dir == "down") {
        directionVec = { 0.f, 1.f };
        sprite.setRotation(180.f);
    }
    else if (dir == "left") {
        directionVec = { -1.f, 0.f };
        sprite.setRotation(270.f);
    }
    else if (dir == "right") {
        directionVec = { 1.f, 0.f };
        sprite.setRotation(90.f);
    }
    else {
        directionVec = { 0.f, 0.f };
        sprite.setRotation(0.f);  // default
    }
}


std::string Vehicle::getDirection() const {
    return currentDirectionStr;
}


void Vehicle::setScale(float scaleX, float scaleY) {
    sprite.setScale(scaleX, scaleY);
}



void Vehicle::setCurrentRoad(const RoadSegment* road) {
    currentRoad = road;
}

const RoadSegment* Vehicle::getCurrentRoad() const {
    return currentRoad;
}

bool Vehicle::isInTurn() const {
    return inTurn;
}

