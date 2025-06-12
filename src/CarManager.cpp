#include "CarManager.h"
#include "ResourceManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <iostream>
using json = nlohmann::json;

CarManager::CarManager() {}

void CarManager::setRoads(const std::vector<RoadSegment>& newRoads) {
    roads = newRoads;
}

void CarManager::addVehicle(const Vehicle& vehicle) {
    vehicles.push_back(vehicle);
}

std::vector<Vehicle>& CarManager::getVehicles() {
    return vehicles;
}

std::vector<RoadSegment>& CarManager::getRoads() {
    return roads;
}

QuadTree<RoadSegment>& CarManager::getRoadTree() {
    return roadTree;
}

void CarManager::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    for (auto& vehicle : vehicles) {
        vehicle.update(dt, blockedPolygons);

        if (vehicle.isInTurn())
            continue;

        int laneIndex = vehicle.getCurrentLaneIndex();
        sf::Vector2f endPoint = vehicle.getCurrentRoad()->getLaneEdge(laneIndex, false);
        float distanceToEnd = length(vehicle.getPosition(), endPoint);

        // ???? ?? ?? ???? ???? ?????
        if (distanceToEnd > 10.f)
            continue;

        sf::Vector2f pos = vehicle.getPosition();
        float queryRange = 100.f;
        sf::FloatRect queryArea(pos.x - queryRange / 2, pos.y - queryRange / 2, queryRange, queryRange);
        std::vector<const RoadSegment*> nearbyRoads;
        roadTree.query(queryArea, nearbyRoads);

        const std::string currentDir = vehicle.getDirection();
        std::vector<const RoadSegment*> possibleTurns;

        for (const RoadSegment* nextRoad : nearbyRoads) {
            if (nextRoad == vehicle.getCurrentRoad())
                continue;

            if (!nextRoad->bounds.contains(pos)) {
                std::string nextDir = nextRoad->direction;

                if (isStraight(currentDir, nextDir) ||
                    isLeftTurn(currentDir, nextDir) ||
                    isRightTurn(currentDir, nextDir)) {

                    int nextLaneIndex = laneIndex;
                    sf::Vector2f fromEdge = vehicle.getCurrentRoad()->getLaneEdge(nextLaneIndex, false);
                    sf::Vector2f toEdge = nextRoad->getLaneEdge(nextLaneIndex, true);
                    float dist = length(fromEdge, toEdge);

                    if (dist < 100.f && isDriveable(*nextRoad, toEdge, nextDir, 50.0f)) {
                        possibleTurns.push_back(nextRoad);
                    }
                }
            }
        }

        if (!possibleTurns.empty()) {
            const RoadSegment* chosen = possibleTurns[0];
            int currentLanes = vehicle.getCurrentRoad()->lanes;
            int nextLanes = chosen->lanes;
            int chosenLaneIndex = laneIndex;
            if (chosenLaneIndex >= nextLanes) chosenLaneIndex = nextLanes - 1;

            sf::Vector2f from = vehicle.getCurrentRoad()->getLaneEdge(chosenLaneIndex, false);
            sf::Vector2f to = chosen->getLaneEdge(chosenLaneIndex, true);
           // to.y = to.y - 2; //change to const Turn_offset

            // --- ????? ??? ?????? ---
            std::string turnType = "straight";
            if (isLeftTurn(currentDir, chosen->direction)) turnType = "left";
            else if (isRightTurn(currentDir, chosen->direction)) turnType = "right";

            // --- ????? ????? control ---
            sf::Vector2f control = calcSmartControlPoint(from, to, currentDir, turnType);
            //sf::Vector2f control(
            //    std::min(from.x, to.x),
            //    std::min(from.y, to.y)
            //);
            std::cout << "control " <<control.x << " " << control.y;
            vehicle.startTurn(from, control, to);
            vehicle.setDirectionVec(getActualLaneDirection(*chosen, chosenLaneIndex));
            vehicle.setCurrentRoad(chosen);
            vehicle.setCurrentLaneIndex(chosenLaneIndex);
        }
    }
}


void CarManager::draw(sf::RenderWindow& window) {
    for (auto& vehicle : vehicles)
        vehicle.draw(window);
}

// NOTE: loadRoadsFromJSON function removed from use! Now using setRoads() instead.

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
   // const RoadSegment& road = roads[roadIdx];
    const RoadSegment& road = roads[0];


    int laneIndex = rand() % std::max(1, road.lanes);
    laneIndex = 1;
    sf::Vector2f laneCenter = road.getLaneCenter(laneIndex);

    float carLength = 50.f;
    float offset = 0.f;

    if (road.direction == "up" || road.direction == "down") {
        float maxOffset = road.bounds.height - carLength;
        if (maxOffset < 0) maxOffset = 0;
        offset = static_cast<float>(rand()) / RAND_MAX * maxOffset;

        if (road.direction == "up")
            laneCenter.y = road.bounds.top + road.bounds.height - offset;
        else
            laneCenter.y = road.bounds.top + offset;
        laneCenter.y = std::clamp(laneCenter.y, road.bounds.top, road.bounds.top + road.bounds.height);
    }
    else {
        float maxOffset = road.bounds.width - carLength;
        if (maxOffset < 0) maxOffset = 0;
        offset = static_cast<float>(rand()) / RAND_MAX * maxOffset;

        if (road.direction == "left")
            laneCenter.x = road.bounds.left + road.bounds.width - offset;
        else
            laneCenter.x = road.bounds.left + offset;
        laneCenter.x = std::clamp(laneCenter.x, road.bounds.left, road.bounds.left + road.bounds.width);
    }

    Vehicle car;
    car.setTexture(ResourceManager::getInstance().getTexture("car"));
    car.setPosition(laneCenter);

    std::string actualDir = getActualLaneDirection(road, laneIndex);
    car.setDirectionVec(actualDir);
    car.setScale(0.05f, 0.05f);
    car.setCurrentRoad(&road);
    car.setCurrentLaneIndex(laneIndex);

    addVehicle(car);

    std::cout << "Spawned car at (" << laneCenter.x << ", " << laneCenter.y << ") on lane "
        << laneIndex << " direction: " << actualDir << " (road#" << roadIdx << ")\n";
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
    else return false;

    for (int i = 1; i <= steps; ++i) {
        sf::Vector2f point = from + dirVec * (length * (float(i) / steps));
        if (!road.bounds.contains(point))
            return false;
    }
    return true;
}

//sf::Vector2f CarManager::getEdgePointOfRoad(const RoadSegment& road, const std::string& direction, int laneIndex, bool entry) {
//    if (direction == "up") {
//        float x = road.bounds.left + road.bounds.width / 2.0f;
//        float y = entry ? road.bounds.top + road.bounds.height : road.bounds.top;
//        return { x, y };
//    }
//    if (direction == "down") {
//        float x = road.bounds.left + road.bounds.width / 2.0f;
//        float y = entry ? road.bounds.top : road.bounds.top + road.bounds.height;
//        return { x, y };
//    }
//    if (direction == "left") {
//        float y = road.bounds.top + road.bounds.height / 2.0f;
//        float x = entry ? road.bounds.left + road.bounds.width : road.bounds.left;
//        return { x, y };
//    }
//    if (direction == "right") {
//        float y = road.bounds.top + road.bounds.height / 2.0f;
//        float x = entry ? road.bounds.left : road.bounds.left + road.bounds.width;
//        return { x, y };
//    }
//    return { road.bounds.left, road.bounds.top };
//}

float CarManager::length(const sf::Vector2f& a, const sf::Vector2f& b) {
    return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

sf::Vector2f CarManager::calcSmartControlPoint(const sf::Vector2f& from, const sf::Vector2f& to, const std::string& dirFrom, const std::string& turnType)
{
    std::cout << " **********************************************************************turn type " << turnType << " from " << dirFrom << std::endl;

    if (turnType == "left") {
        if (dirFrom == "right")
            return { from.x, to.y };
        if (dirFrom == "left")
            return { to.x, from.y };
        if (dirFrom == "down")
            return { from.x, to.y};
        if (dirFrom == "up")
            return { to.x, from.y };
    }
    if (turnType == "right") {
        if (dirFrom == "right")
            return { from.x, to.y };
        if (dirFrom == "left")
            return { from.x, to.y };
        if (dirFrom == "down")
            return { to.x, from.y };
        if (dirFrom == "up")
            //sf::Vector2f control(
//    std::min(from.x, to.x),
//    std::min(from.y, to.y)
//);
            return { std::min(from.x, to.x), std::min(from.y, to.y)  + 2};
    }
  
    // ???/????? ????: ???? ????
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
