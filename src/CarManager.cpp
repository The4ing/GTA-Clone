#include "CarManager.h"
#include "ResourceManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <iostream>
using json = nlohmann::json;

CarManager::CarManager() {}

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

void CarManager::update(float dt) {
    for (auto& vehicle : vehicles) {
        vehicle.update(dt);

        if (vehicle.isInTurn())
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

                    int laneIndex = vehicle.getCurrentLaneIndex();
                    sf::Vector2f fromEdge = getEdgePointOfRoad(*vehicle.getCurrentRoad(), currentDir, laneIndex, false);
                    sf::Vector2f toEdge = getEdgePointOfRoad(*nextRoad, nextDir, laneIndex, true);
                    float dist = length(fromEdge, toEdge);
                    sf::Vector2f toEdger = getEdgePointOfRoad(*nextRoad, nextDir, laneIndex, false);
                    float distEnd = length(fromEdge, toEdger);
                    float minValue = std::min(dist, distEnd);

                    if (minValue < 100.f && isDriveable(*nextRoad, toEdge, nextDir, 50.0f)) {
                        possibleTurns.push_back(nextRoad);
                    }
                }
            }
        }

        if (!possibleTurns.empty()) {
            const RoadSegment* chosen = possibleTurns[0];
            int currentLanes = vehicle.getCurrentRoad()->lanes;
            int nextLanes = chosen->lanes;
            int laneIndex = vehicle.getCurrentLaneIndex();
            if (laneIndex >= nextLanes) laneIndex = nextLanes - 1;

            sf::Vector2f from = vehicle.getCurrentRoad()->getLaneEdge(laneIndex, false);
            sf::Vector2f to = chosen->getLaneEdge(laneIndex, true);
            sf::Vector2f control = calcSmartControlPoint(from, to, currentDir, chosen->direction);

            vehicle.startTurn(from, control, to);
            vehicle.setDirectionVec(getActualLaneDirection(*chosen, laneIndex));
            vehicle.setCurrentRoad(chosen);
            vehicle.setCurrentLaneIndex(laneIndex);
        }
    }
}

void CarManager::draw(sf::RenderWindow& window) {
    for (auto& vehicle : vehicles)
        vehicle.draw(window);
}

void CarManager::loadRoadsFromJSON(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open " << filename << std::endl;
        return;
    }

    json data;
    file >> data;

    roads.clear();

    for (const auto& layer : data["layers"]) {
        if (layer["type"] == "objectgroup" && (layer["name"] == "roads")) {
            for (const auto& obj : layer["objects"]) {
                if (obj.contains("properties")) {
                    RoadSegment road;
                    road.bounds.left = obj["x"];
                    road.bounds.top = obj["y"];
                    road.bounds.width = obj["width"];
                    road.bounds.height = obj["height"];

                    for (const auto& prop : obj["properties"]) {
                        std::string name = prop["name"];
                        if (name == "Direction")
                            road.direction = prop["value"];
                        else if (name == "Lanes")
                            road.lanes = prop["value"];
                        else if (name == "2D")
                            road.is2D = prop["value"];
                    }

                    if (!road.direction.empty())
                        roads.push_back(road);
                }
            }
        }
    }
    std::cout << "Loaded " << roads.size() << " road segments\n";
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
    // ??? ??? ???? ?????
    int roadIdx = rand() % roads.size();
    const RoadSegment& road = roads[roadIdx];

    // ??? ???? ?????
    int laneIndex = rand() % std::max(1, road.lanes);

    // ??? ????? ?????? ?? ????? (????? ?????, ?? ??? ????? ?????)
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


// ---- ???????? ??? ----
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

sf::Vector2f CarManager::getEdgePointOfRoad(const RoadSegment& road, const std::string& direction, int laneIndex, bool entry) {
    if (direction == "up") {
        float x = road.bounds.left + road.bounds.width / 2.0f;
        float y = entry ? road.bounds.top + road.bounds.height : road.bounds.top;
        return { x, y };
    }
    if (direction == "down") {
        float x = road.bounds.left + road.bounds.width / 2.0f;
        float y = entry ? road.bounds.top : road.bounds.top + road.bounds.height;
        return { x, y };
    }
    if (direction == "left") {
        float y = road.bounds.top + road.bounds.height / 2.0f;
        float x = entry ? road.bounds.left + road.bounds.width : road.bounds.left;
        return { x, y };
    }
    if (direction == "right") {
        float y = road.bounds.top + road.bounds.height / 2.0f;
        float x = entry ? road.bounds.left : road.bounds.left + road.bounds.width;
        return { x, y };
    }
    return { road.bounds.left, road.bounds.top };
}

float CarManager::length(const sf::Vector2f& a, const sf::Vector2f& b) {
    return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

sf::Vector2f CarManager::calcSmartControlPoint(
    const sf::Vector2f& from,
    const sf::Vector2f& to,
    const std::string& dirFrom,
    const std::string& dirTo)
{
    if (dirFrom == dirTo) {
        return (from + to) * 0.5f;
    }
    else {
        if (std::abs(from.x - to.x) > std::abs(from.y - to.y)) {
            return { from.x, to.y };
        }
        else {
            return { to.x, from.y };
        }
    }
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
