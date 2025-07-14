#include "Vehicle.h"
#include "ResourceManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <cmath>
#include <iostream>
#include "Player.h" 
#include <algorithm> // for std::min/max
#include "Constants.h"
#include "CollisionUtils.h"


using json = nlohmann::json;

Vehicle::Vehicle() : m_driver(nullptr), parking(false) {
    sprite.setTexture(ResourceManager::getInstance().getTexture("car_sheet"));
    sprite.setTextureRect(sf::IntRect(0, 0, 600, 600));

    // Set origin to center for rotation
    sf::FloatRect bounds = sprite.getLocalBounds();
    sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

    position = { 300.f, 300.f };
    sprite.setPosition(position);
    // Initialize angle based on initial directionVec if needed, or default to 0
    // For now, player will control angle directly. AI uses directionVec.
    angle = sprite.getRotation(); // Assuming initial rotation is set by setDirectionVec if AI
}

void Vehicle::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    if (stopTimer > 0.f) {
        stopTimer -= dt;
        if (stopTimer < 0.f)
            stopTimer = 0.f;
        speed = 0.f;
        sprite.setPosition(position);
        return;
    }
    if (inTurn) {
        // U-turn / Bezier curve logic from first function
        bezierT += dt * 0.7f;
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
    else if (m_driver) {
        // Player driving logic from second function

        // Speed decay / friction
        if (speed > 0) {
            speed -= decelerationRate * 0.3f * dt;
            if (speed < 0) speed = 0;
        }
        else if (speed < 0) {
            speed += decelerationRate * 0.3f * dt;
            if (speed > 0) speed = 0;
        }

        // Calculate direction vector from angle
        float radAngle = (angle - 90.f) * M_PI / 180.f;
        sf::Vector2f currentDir(std::cos(radAngle), std::sin(radAngle));

        // Calculate next position
        sf::Vector2f nextPosition = position + currentDir * speed * dt;

        // Collision detection
        bool collisionDetected = false;
        if (speed != 0.f) {
            sf::Sprite nextSprite = sprite;
            nextSprite.setPosition(nextPosition);
            nextSprite.setRotation(angle);

            // Get transformed hitbox polygon at next position
            sf::Transform nextTransform = nextSprite.getTransform();
            sf::FloatRect localBounds = sprite.getLocalBounds();

            const float widthShrinkFactor = 0.3f;
            const float heightShrinkFactor = 0.8f;
            const float shrinkX = (1.f - widthShrinkFactor) * localBounds.width / 2.f;
            const float shrinkY = (1.f - heightShrinkFactor) * localBounds.height / 2.f;

            std::vector<sf::Vector2f> hitbox = {
                nextTransform.transformPoint({ localBounds.left + shrinkX, localBounds.top + shrinkY }),
                nextTransform.transformPoint({ localBounds.left + localBounds.width - shrinkX, localBounds.top + shrinkY }),
                nextTransform.transformPoint({ localBounds.left + localBounds.width - shrinkX, localBounds.top + localBounds.height - shrinkY }),
                nextTransform.transformPoint({ localBounds.left + shrinkX, localBounds.top + localBounds.height - shrinkY })
            };

            for (const auto& point : hitbox) {
                if (CollisionUtils::isInsideBlockedPolygon(point, blockedPolygons)) {
                    collisionDetected = true;
                    break;
                }
            }
        }


        if (!collisionDetected) {
            position = nextPosition;
        }
        else {
            speed = 0; // Stop on collision
        }

        sprite.setPosition(position);
        sprite.setRotation(angle);
    }
    else {
        // AI straight movement logic, only if derived class doesn't handle its own rotation
        if (!parking && !handlesOwnAIRotation()) {
            // AI straight movement logic from first function
            sf::Vector2f dir = directionVec;
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len > 0) dir /= len;

            position += dir * speed * dt;
            sprite.setPosition(position);

            float angle = std::atan2(dir.y, dir.x) * 180.f / 3.14159f + 90.f;
            sprite.setRotation(angle);
        }
    }
}


void Vehicle::draw(sf::RenderTarget& target) {
    target.draw(sprite);

    // Draw hitbox polygon for debugging similar to PoliceCar
    std::vector<sf::Vector2f> corners = getHitboxPolygon();
    sf::Vertex outline[] = {
        sf::Vertex(corners[0], sf::Color::Red),
        sf::Vertex(corners[1], sf::Color::Red),
        sf::Vertex(corners[2], sf::Color::Red),
        sf::Vertex(corners[3], sf::Color::Red),
        sf::Vertex(corners[0], sf::Color::Red)
    };

    target.draw(outline, 5, sf::LineStrip);
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



std::vector<sf::Vector2f> Vehicle::getHitboxPolygon() const {
    sf::FloatRect localBounds = sprite.getLocalBounds();
    sf::Transform transform = sprite.getTransform();

    const float widthShrinkFactor = 0.3f;   // 70% ??????
    const float heightShrinkFactor = 0.8f;  // 80% ??????

    const float shrinkX = (1.f - widthShrinkFactor) * localBounds.width / 2.f;
    const float shrinkY = (1.f - heightShrinkFactor) * localBounds.height / 2.f;

    std::vector<sf::Vector2f> corners;
    corners.push_back(transform.transformPoint({ localBounds.left + shrinkX, localBounds.top + shrinkY }));
    corners.push_back(transform.transformPoint({ localBounds.left + localBounds.width - shrinkX, localBounds.top + shrinkY }));
    corners.push_back(transform.transformPoint({ localBounds.left + localBounds.width - shrinkX, localBounds.top + localBounds.height - shrinkY }));
    corners.push_back(transform.transformPoint({ localBounds.left + shrinkX, localBounds.top + localBounds.height - shrinkY }));

    return corners;
}

void Vehicle::stopForSeconds(float seconds) {
    stop();
    stopTimer = std::max(stopTimer, seconds);
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
    //std::cout << "[START TURN] from: " << from.x << "," << from.y
    //    << " control: " << control.x << "," << control.y
    //    << " to: " << to.x << "," << to.y << std::endl;
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

void Vehicle::setTextureRect(const sf::IntRect& rect) {
    sprite.setTextureRect(rect);
    sf::FloatRect bounds = sprite.getLocalBounds();
    sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
}

void Vehicle::stop() {
    speed = 0.f;
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

// Player control method implementations
void Vehicle::accelerate(float dt) {
    if (m_driver) {
        speed += accelerationRate * dt;
        speed = std::min(speed, maxSpeed);
    }
}

void Vehicle::brake(float dt) {
    if (m_driver) {
        if (speed > 0) {
            speed -= decelerationRate * dt;
            speed = std::max(speed, 0.f); // Don't go into reverse from braking forward motion
        }
        else { // Allow braking to become reversing if already stopped or moving backward
            speed -= accelerationRate * dt * 0.7f; // Slower acceleration for reverse
            speed = std::max(speed, reverseSpeed);
        }
    }
}

void Vehicle::steerLeft(float dt) {
    if (m_driver && std::abs(speed) > 0.1f) { // Allow steering only if moving
        angle -= turnRate * dt * (speed / maxSpeed); // Steering input less effective at low speeds
        if (angle < 0.f) angle += 360.f;
    }
}

void Vehicle::steerRight(float dt) {
    if (m_driver && std::abs(speed) > 0.1f) { // Allow steering only if moving
        angle += turnRate * dt * (speed / maxSpeed); // Steering input less effective at low speeds
        if (angle >= 360.f) angle -= 360.f;
    }
}

void Vehicle::setDriver(Player* driver) {
    m_driver = driver;
    if (m_driver) {
        // When player enters, sync vehicle's angle with its current sprite rotation
        // if it was previously AI controlled.
        // Or, reset speed and angle for player control.
        // For now, let's reset speed. Angle will be taken from sprite.
        speed = 0.f; // Reset speed when player takes over
        angle = sprite.getRotation(); // Sync angle with current visual rotation
        inTurn = false; // Player control overrides AI turning
    }
    else {
        parking = true;
        // Player exited, potentially hand back to AI.
        // AI logic in CarManager would need to re-engage pathfinding.
        // For now, vehicle just stops if player exits.
        // speed = 0.f; // Or let it coast, or let AI take over.
        // The AI's setDirectionVec would set the sprite rotation appropriately.
    }
}

Player* Vehicle::getDriver() const {
    return m_driver;
}

bool Vehicle::hasDriver() const {
    return m_driver != nullptr;
}

sf::Sprite& Vehicle::getSprite() {
    return sprite;
}

const sf::Sprite& Vehicle::getSprite() const {
    return sprite;
}

bool Vehicle::isDestroyed() const {
    return destroyed;
}

void Vehicle::setDestroyed(bool value) {
    destroyed = value;
}
