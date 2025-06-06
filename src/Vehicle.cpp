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
        bezierT += dt * 0.7f;  // ????? ?????? ????? ???
        if (bezierT >= 1.f) {
            bezierT = 1.f;
            inTurn = false;
            setDirectionVec(currentDirectionStr);

            if (currentRoad) {
                setPosition(currentRoad->getLaneEdge(currentLaneIndex, true));
                sprite.setPosition(position);
            }
        }

        float t = bezierT;
        float oneMinusT = 1.f - t;
        sf::Vector2f pos =
            oneMinusT * oneMinusT * bezierP0 +
            2.f * oneMinusT * t * bezierP1 +
            t * t * bezierP2;

        sf::Vector2f tangent =
            2.f * (1.f - t) * (bezierP1 - bezierP0) +
            2.f * t * (bezierP2 - bezierP1);

        float angle = std::atan2(tangent.y, tangent.x) * 180.f / 3.14159f + 90.f;
        sprite.setRotation(angle);

        position = pos;
        sprite.setPosition(pos);
    }
    else {
        // ????? ????
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

    if (inTurn) {
        sf::CircleShape startPoint(5.f);
        startPoint.setFillColor(sf::Color::Green);
        startPoint.setOrigin(5.f, 5.f);
        startPoint.setPosition(bezierP0);

        sf::CircleShape controlPoint(5.f);
        controlPoint.setFillColor(sf::Color::Blue);
        controlPoint.setOrigin(5.f, 5.f);
        controlPoint.setPosition(bezierP1);

        sf::CircleShape endPoint(5.f);
        endPoint.setFillColor(sf::Color::Red);
        endPoint.setOrigin(5.f, 5.f);
        endPoint.setPosition(bezierP2);

        target.draw(startPoint);
        target.draw(controlPoint);
        target.draw(endPoint);

        sf::Vertex line1[] = {
            sf::Vertex(bezierP0, sf::Color::White),
            sf::Vertex(bezierP1, sf::Color::White)
        };
        sf::Vertex line2[] = {
            sf::Vertex(bezierP1, sf::Color::White),
            sf::Vertex(bezierP2, sf::Color::White)
        };
        target.draw(line1, 2, sf::Lines);
        target.draw(line2, 2, sf::Lines);

        for (float t = 0.f; t < 1.f; t += 0.02f) {
            sf::Vector2f pointA = bezier(t, bezierP0, bezierP1, bezierP2);
            sf::Vector2f pointB = bezier(t + 0.02f, bezierP0, bezierP1, bezierP2);
            sf::Vertex curve[] = { sf::Vertex(pointA, sf::Color::Yellow), sf::Vertex(pointB, sf::Color::Yellow) };
            target.draw(curve, 2, sf::Lines);
        }
    }
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
    std::cout << "[START TURN] from: " << from.x << "," << from.y
        << " control: " << control.x << "," << control.y
        << " to: " << to.x << "," << to.y << std::endl;
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
    currentDirectionStr = dir;
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
        sprite.setRotation(0.f);  // ????? ????
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

sf::Vector2f Vehicle::bezier(float t, const sf::Vector2f& P0, const sf::Vector2f& P1, const sf::Vector2f& P2) {
    float oneMinusT = 1.f - t;
    return oneMinusT * oneMinusT * P0 +
        2 * oneMinusT * t * P1 +
        t * t * P2;
}
