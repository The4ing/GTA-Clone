#include "CarManager.h"
#include "ResourceManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <algorithm> // std::shuffle
#include <random>    // std::default_random_engine
#include "PoliceManager.h" 
#include <memory>
using json = nlohmann::json;

CarManager::CarManager(PoliceManager& policeMgr)
    : vehicleTree(sf::FloatRect(0.f, 0.f, 4640.f, 4672.f)),
    m_policeManager(policeMgr) {
    if (!debugFont.loadFromFile("resources/Miskan.ttf")) {
        std::cerr << "Failed to load debug font for road debug text\n";
    }
    // Seed for random number generation if not already done elsewhere (e.g. main or GameManager)
    // std::srand(static_cast<unsigned int>(std::time(nullptr))); // Example

}


void CarManager::setRoads(const std::vector<RoadSegment>& newRoads) {
    roads = newRoads;
}

void CarManager::addVehicle(std::unique_ptr<Vehicle> vehicle) {
    vehicles.push_back(std::move(vehicle));
}

std::vector<std::unique_ptr<Vehicle>>& CarManager::getVehicles() {
    return vehicles;
}

std::vector<RoadSegment>& CarManager::getRoads() {
    return roads;
}

QuadTree<RoadSegment>& CarManager::getRoadTree() {
    return roadTree;
}

void CarManager::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons, const sf::View& view) {
    int nextLaneIndex;
    bool turnFound;

    vehicleTree.clear();
    for (auto& vPtr : vehicles) {
        Vehicle& v = *vPtr;
        sf::Vector2f pos = v.getPosition();
        sf::FloatRect area(pos.x - 0.5f, pos.y - 0.5f, 1.f, 1.f);
        vehicleTree.insert(area, vPtr.get());
    }

    for (auto& vehiclePtr : vehicles) {
        Vehicle& vehicle = *vehiclePtr;
        // First, call the vehicle's own update method.
        // This handles player input if present, or basic AI movement (like Bezier curve execution if inTurn is true for AI).
        vehicle.update(dt, blockedPolygons);
        if (!vehicle.hasDriver() && vehicle.getSpeed() < 0.1f)
            vehicle.setDestroyed(true);
        // If a player is driving this vehicle, skip all AI decision-making and pathfinding logic.
        if (vehicle.hasDriver()) {
            continue;
        }

        sf::Vector2f pos = vehicle.getPosition();
        sf::FloatRect area(pos.x - 30.f, pos.y - 30.f, 60.f, 60.f);
        std::vector<Vehicle*> nearby;
        vehicleTree.query(area, nearby);

        bool hasCollision = false;
        sf::FloatRect myBounds = vehicle.getSprite().getGlobalBounds();
        for (auto* other : nearby) {
            if (other == &vehicle) continue;
            if (other->hasDriver()) continue;

            if (myBounds.intersects(other->getSprite().getGlobalBounds())) {
                vehicle.stopForSeconds(1.f); // ?? ???? ??? ????
                hasCollision = true;
                break;
            }

        }
        if (hasCollision) {
            continue;
        }

        // vehicle.update(dt, blockedPolygons);

        if (vehicle.isInTurn()) { // If AI vehicle is executing a Bezier turn set by previous AI logicAdd commentMore actions
            continue; // Let the turn complete (handled by vehicle.update() if AI and inTurn)
        }

        int laneIndex = vehicle.getCurrentLaneIndex();
        sf::Vector2f endPoint = vehicle.getCurrentRoad()->getLaneEdge(laneIndex, false);
        float distanceToEnd = length(vehicle.getPosition(), endPoint);
        if (distanceToEnd > 10.f)
            continue;

        float queryRange = 160.f;
        sf::FloatRect queryArea(pos.x - queryRange / 2, pos.y - queryRange / 2, queryRange, queryRange);
        std::vector<const RoadSegment*> nearbyRoads;
        roadTree.query(queryArea, nearbyRoads);
        std::shuffle(nearbyRoads.begin(), nearbyRoads.end(), std::mt19937{ std::random_device{}() });

        const std::string currentDir = vehicle.getDirection();
        turnFound = false;

        for (const RoadSegment* nextRoad : nearbyRoads) {
            if (nextRoad == vehicle.getCurrentRoad()) {
                rejectedRoads.emplace_back(nextRoad, "same road");
                continue;
            }

            int nextLanes = nextRoad->lanes;
            nextLaneIndex = rand() % std::max(1, nextLanes);

            std::string actualNextDir = getActualLaneDirection(*nextRoad, nextLaneIndex);
            if (!(isStraight(currentDir, actualNextDir) ||
                isLeftTurn(currentDir, actualNextDir) ||
                isRightTurn(currentDir, actualNextDir))) {
                rejectedRoads.emplace_back(nextRoad, "invalid direction from " + currentDir + " to " + actualNextDir);
                continue;
            }

            sf::Vector2f startEdge = nextRoad->getLaneEdge(nextLaneIndex, true);
            sf::Vector2f endEdge = nextRoad->getLaneEdge(nextLaneIndex, false);
            sf::Vector2f fromEdge = vehicle.getCurrentRoad()->getLaneEdge(laneIndex, false);

            float distToStart = length(fromEdge, startEdge);
            float distToEnd = length(fromEdge, endEdge);
            bool entryAtStart = distToStart < distToEnd;
            sf::Vector2f toEdge = nextRoad->getLaneEdge(nextLaneIndex, entryAtStart);

            sf::Vector2f dirVec;
            if (actualNextDir == "right") dirVec = { 1.f, 0.f };
            else if (actualNextDir == "left") dirVec = { -1.f, 0.f };
            else if (actualNextDir == "up") dirVec = { 0.f, -1.f };
            else if (actualNextDir == "down") dirVec = { 0.f, 1.f };

            forwardPoint = toEdge + dirVec * 30.f;
            if (!nextRoad->bounds.contains(forwardPoint)) {
                rejectedRoads.emplace_back(nextRoad, "turn lands too close to end of road");
                continue;
            }

            if (!isDriveable(*nextRoad, toEdge, actualNextDir, 50.f)) {
                rejectedRoads.emplace_back(nextRoad, "not driveable");
                continue;
            }

            std::string turnType = "straight";
            if (isLeftTurn(currentDir, actualNextDir)) turnType = "left";
            else if (isRightTurn(currentDir, actualNextDir)) turnType = "right";

            sf::Vector2f control = calcSmartControlPoint(fromEdge, toEdge, currentDir, turnType);

            turnFound = true;
            vehicle.startTurn(fromEdge, control, toEdge);
            vehicle.setDirectionVec(actualNextDir);
            vehicle.setPreviousRoad(vehicle.getCurrentRoad());
            vehicle.setCurrentRoad(nextRoad);
            vehicle.setCurrentLaneIndex(nextLaneIndex);
            break;
        }

        if (!turnFound) {
            const RoadSegment* currentRoad = vehicle.getCurrentRoad();
            int lanes = currentRoad->lanes;
            int oldLane = vehicle.getCurrentLaneIndex();
            int newLane = lanes - 1 - oldLane;

            std::string currentDir = vehicle.getDirection();
            std::string reverseDir;
            if (currentDir == "up") reverseDir = "down";
            else if (currentDir == "down") reverseDir = "up";
            else if (currentDir == "left") reverseDir = "right";
            else if (currentDir == "right") reverseDir = "left";

            sf::Vector2f dirVec;
            if (currentDir == "up") dirVec = { 0.f, -1.f };
            else if (currentDir == "down") dirVec = { 0.f, 1.f };
            else if (currentDir == "left") dirVec = { -1.f, 0.f };
            else if (currentDir == "right") dirVec = { 1.f, 0.f };

            sf::Vector2f from = vehicle.getPosition();
            sf::Vector2f to = currentRoad->getLaneEdge(newLane, true);
            to = to - dirVec * 50.f;

            sf::Vector2f control = (from + to) * 0.5f + dirVec * 60.f;

            vehicle.startTurn(from, control, to);
            vehicle.setDirectionVec(reverseDir);
            vehicle.setCurrentLaneIndex(newLane);
            vehicle.setPreviousRoad(currentRoad);
            vehicle.setCurrentRoad(currentRoad);
        }
    }
    sf::FloatRect viewRect(view.getCenter() - view.getSize() / 2.f, view.getSize());

    auto it = vehicles.begin();
    while (it != vehicles.end()) {
        Vehicle& v = **it;
        if (v.isDestroyed() && !viewRect.intersects(v.getSprite().getGlobalBounds())) {
            it = vehicles.erase(it);
            spawnVehicleOffScreen(view);
        }
        else {
            ++it;
        }
    }
}

void CarManager::spawnVehicleOffScreen(const sf::View& view) {
    if (roads.empty()) return;

    sf::FloatRect viewRect(view.getCenter() - view.getSize() / 2.f, view.getSize());
    for (int attempt = 0; attempt < 50; ++attempt) {
        int roadIdx = rand() % roads.size();
        const RoadSegment& road = roads[roadIdx];
        if (viewRect.intersects(road.bounds))
            continue;

        int laneIndex = rand() % std::max(1, road.lanes);
        sf::Vector2f laneCenter = road.getLaneCenter(laneIndex);

        float carLength = 50.f;
        float offset = 0.f;

        if (road.direction == "up" || road.direction == "down") {
            float maxOffset = road.bounds.height - carLength;
            offset = static_cast<float>(rand()) / RAND_MAX * std::max(0.f, maxOffset);
            laneCenter.y = (road.direction == "up") ? road.bounds.top + road.bounds.height - offset
                : road.bounds.top + offset;
        }
        else {
            float maxOffset = road.bounds.width - carLength;
            offset = static_cast<float>(rand()) / RAND_MAX * std::max(0.f, maxOffset);
            laneCenter.x = (road.direction == "left") ? road.bounds.left + road.bounds.width - offset
                : road.bounds.left + offset;
        }

        auto car = std::make_unique<Vehicle>();
        car->setTexture(ResourceManager::getInstance().getTexture("car_sheet"));
        int carIndex = rand() % 7;
        int frameWidth = 600;
        int frameHeight = 600;
        int columns = 3;
        int col = carIndex % columns;
        int row = carIndex / columns;
        car->setTextureRect(sf::IntRect(col * frameWidth, row * frameHeight, frameWidth, frameHeight));

        car->setPosition(laneCenter);
        car->setScale(0.05f, 0.05f);
        std::string actualDir = getActualLaneDirection(road, laneIndex);
        car->setDirectionVec(actualDir);
        car->setCurrentRoad(&road);
        car->setCurrentLaneIndex(laneIndex);

        addVehicle(std::move(car));
        vehicleTree.insert(sf::FloatRect(laneCenter.x, laneCenter.y, 1.f, 1.f), vehicles.back().get());
        return;
    }
}


void CarManager::draw(sf::RenderTarget& window) {
    // Draw rejected roads in gray with yellow outline and reason text
    for (const auto& [road, reason] : rejectedRoads) {
        sf::RectangleShape rect;
        rect.setPosition(road->bounds.left, road->bounds.top);
        rect.setSize({ road->bounds.width, road->bounds.height });
        rect.setFillColor(sf::Color(60, 60, 60, 100)); // Gray + transparent
        rect.setOutlineColor(sf::Color::Yellow);
        rect.setOutlineThickness(1.5f);
        window.draw(rect);

        // Text label of reason
        if (debugFont.getInfo().family != "") {
            sf::Text text;
            text.setFont(debugFont);
            text.setString(reason);
            text.setCharacterSize(10);
            text.setFillColor(sf::Color::White);
            text.setPosition(road->bounds.left + 2.f, road->bounds.top + 2.f);
            window.draw(text);
        }
    }

    for (auto& vehiclePtr : vehicles) {
        Vehicle& vehicle = *vehiclePtr;
        // Highlight current road in blue
        const RoadSegment* current = vehicle.getCurrentRoad();
        if (current) {
            sf::RectangleShape rect;
            rect.setPosition(current->bounds.left, current->bounds.top);
            rect.setSize({ current->bounds.width, current->bounds.height });
            rect.setFillColor(sf::Color::Transparent);
            rect.setOutlineColor(sf::Color::Blue);
            rect.setOutlineThickness(2.f);
            window.draw(rect);
        }

        // Highlight previous road in red if vehicle is turning
        if (vehicle.isInTurn()) {
            const RoadSegment* prev = vehicle.getPreviousRoad();
            if (prev) {
                sf::RectangleShape rect;
                rect.setPosition(prev->bounds.left, prev->bounds.top);
                rect.setSize({ prev->bounds.width, prev->bounds.height });
                rect.setFillColor(sf::Color::Transparent);
                rect.setOutlineColor(sf::Color::Red);
                rect.setOutlineThickness(2.f);
                window.draw(rect);
            }
        }
        for (const auto& dp : driveableDebugPoints) {
            sf::CircleShape dot(2.f);
            dot.setOrigin(2.f, 2.f);
            dot.setPosition(dp.pos);
            dot.setFillColor(dp.valid ? sf::Color::Green : sf::Color::Red);
            window.draw(dot);
        }

        sf::CircleShape dot(6.f);
        dot.setOrigin(6.f, 6.f);
        dot.setPosition(forwardPoint);
        dot.setFillColor(sf::Color::Red);
        window.draw(dot);
        vehicle.draw(window);
    }
}

void CarManager::buildRoadTree() {
    roadTree = QuadTree<RoadSegment>(sf::FloatRect(0, 0, 4640, 4672));
    for (const auto& road : roads) {
        roadTree.insert(road.bounds, road);
    }
}

std::vector<RoadSegment> CarManager::findNearbyRoads(const sf::FloatRect& area) {
    return roadTree.query(area);
}

void CarManager::spawnSingleVehicleOnRoad() {
    if (roads.empty()) {
        std::cerr << "No roads loaded to place vehicle.\n";
        return;
    }

    int roadIdx = rand() % roads.size();
    roadIdx = 0;
    const RoadSegment& road = roads[roadIdx];
    int laneIndex = rand() % std::max(1, road.lanes);
    sf::Vector2f laneCenter = road.getLaneCenter(laneIndex);

    float carLength = 50.f;
    float offset = 0.f;

    if (road.direction == "up" || road.direction == "down") {
        float maxOffset = road.bounds.height - carLength;
        offset = static_cast<float>(rand()) / RAND_MAX * std::max(0.f, maxOffset);
        laneCenter.y = (road.direction == "up") ? road.bounds.top + road.bounds.height - offset
            : road.bounds.top + offset;
    }
    else {
        float maxOffset = road.bounds.width - carLength;
        offset = static_cast<float>(rand()) / RAND_MAX * std::max(0.f, maxOffset);
        laneCenter.x = (road.direction == "left") ? road.bounds.left + road.bounds.width - offset
            : road.bounds.left + offset;
    }

    auto car = std::make_unique<Vehicle>();
    car->setTexture(ResourceManager::getInstance().getTexture("car_sheet"));
    int carIndex = rand() % 7; // 7 car variants in the sprite sheet
    int frameWidth = 600;
    int frameHeight = 600;
    int columns = 3;
    int col = carIndex % columns;
    int row = carIndex / columns;
    car->setTextureRect(sf::IntRect(col * frameWidth, row * frameHeight, frameWidth, frameHeight));

    car->setPosition(laneCenter);
    car->setScale(0.05f, 0.05f);
    std::string actualDir = getActualLaneDirection(road, laneIndex);
    car->setDirectionVec(actualDir);
    car->setCurrentRoad(&road);
    car->setCurrentLaneIndex(laneIndex);
    //addVehicle(car);
    //vehicleTree.insert(sf::FloatRect(laneCenter.x, laneCenter.y, 1.f, 1.f), &vehicles.back());


    //std::cout << "Spawned car at (" << laneCenter.x << ", " << laneCenter.y
    //    << ") on lane " << laneIndex << " direction: " << actualDir
    //    << " (road#" << roadIdx << ")\n";
   // int spawnChoice = rand() % 100; // Random number between 0 and 99Add commentMore actions


    //if (spawnChoice < POLICE_CAR_SPAWN_CHANCE) {
    //    // Spawn an ambient police car via PoliceManager
    //    // The method signature for spawnAmbientPoliceCarOnRoadSegment is defined in plan step 5
    //    // as (const RoadSegment* road, int laneIndex, const std::string& actualDir, const sf::Vector2f& spawnPosition)
    //    m_policeManager.spawnAmbientPoliceCarOnRoadSegment(&road, laneIndex, actualDir, laneCenter);
    //    // std::cout << "Attempted to spawn ambient POLICE car at (" << laneCenter.x << ", " << laneCenter.y
    //    //           << ") on lane " << laneIndex << " direction: " << actualDir
    //    //           << " (road#" << roadIdx << ")\n";
    //}
    //else {
        // Spawn a regular car
        addVehicle(std::move(car));
        vehicleTree.insert(sf::FloatRect(laneCenter.x, laneCenter.y, 1.f, 1.f), vehicles.back().get());
        // std::cout << "Spawned REGULAR car at (" << laneCenter.x << ", " << laneCenter.y
        //           << ") on lane " << laneIndex << " direction: " << actualDir
        //           << " (road#" << roadIdx << ")\n";
    //}
}


bool CarManager::isRightTurn(const std::string& from, const std::string& to) {
    return (from == "up" && to == "right") ||
        (from == "right" && to == "down") ||
        (from == "down" && to == "left") ||
        (from == "left" && to == "up");
}

bool CarManager::isLeftTurn(const std::string& from, const std::string& to) {
    return (from == "up" && to == "left") ||
        (from == "left" && to == "down") ||
        (from == "down" && to == "right") ||
        (from == "right" && to == "up");
}

bool CarManager::isStraight(const std::string& from, const std::string& to) {
    return (from == to);
}

bool CarManager::isDriveable(const RoadSegment& road, sf::Vector2f from, const std::string& direction, float length) {

    const int steps = 10;
    sf::Vector2f dirVec;
    if (direction == "right")      dirVec = { 1, 0 };
    else if (direction == "left")  dirVec = { -1, 0 };
    else if (direction == "up")    dirVec = { 0, -1 };
    else if (direction == "down")  dirVec = { 0, 1 };
    else {
        return false;
    }

    for (int i = 2; i <= steps; ++i) {
        sf::Vector2f point = from + dirVec * (length * (float(i) / steps));
        bool valid = road.bounds.contains(point);
        driveableDebugPoints.push_back({ point, valid });

        if (!valid) {
            return false;
        }
    }

    return true;
}


float CarManager::length(const sf::Vector2f& a, const sf::Vector2f& b) {
    return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

sf::Vector2f CarManager::calcSmartControlPoint(const sf::Vector2f& from, const sf::Vector2f& to, const std::string& dirFrom, const std::string& turnType)
{
    if (turnType == "left") {
        if (dirFrom == "right")
            return { std::max(from.x, to.x), std::max(from.y, to.y) };
        if (dirFrom == "left")
            return { std::min(from.x, to.x), std::min(from.y, to.y) };
        if (dirFrom == "down")
            return { std::min(from.x, to.x), std::max(from.y, to.y) };
        if (dirFrom == "up")
            return { std::max(from.x, to.x), std::min(from.y, to.y) };
    }
    if (turnType == "right") {
        if (dirFrom == "right")
            return { std::max(from.x, to.x), std::min(from.y, to.y) };
        if (dirFrom == "left")
            return { std::min(from.x, to.x), std::max(from.y, to.y) };
        if (dirFrom == "down")
            return { std::max(from.x, to.x), std::max(from.y, to.y) };
        if (dirFrom == "up")
            return { std::min(from.x, to.x), std::min(from.y, to.y) };
    }
    return (from + to) * 0.5f;
}


std::string CarManager::getActualLaneDirection(const RoadSegment& road, int laneIndex) {
    if (!road.is2D) return road.direction;
    int half = road.lanes / 2;
    if (laneIndex < half) {
        if (road.direction == "right") return "left";
        if (road.direction == "left") return "right";
        if (road.direction == "up") return "down";
        if (road.direction == "down") return "up";
    }
    return road.direction;
}