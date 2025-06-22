#include "CarManager.h"
#include "ResourceManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <algorithm> // std::shuffle
#include <random>    // std::default_random_engine

using json = nlohmann::json;

CarManager::CarManager() : vehicleTree(sf::FloatRect(0.f, 0.f, 4640.f, 4672.f)) {
    if (!debugFont.loadFromFile("resources/Miskan.ttf")) {
        std::cerr << "Failed to load debug font for road debug text\n";
    }

}


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

//void CarManager::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
//    int nextLaneIndex;
//    rejectedRoads.clear();
//
//    for (auto& vehicle : vehicles) {
//        for (auto& vehicle : vehicles) {
//            sf::Vector2f pos = vehicle.getPosition();
//            sf::FloatRect area(pos.x - 30.f, pos.y - 30.f, 60.f, 60.f);
//            std::vector<Vehicle*> nearby;
//            vehicleTree.query(area, nearby);
//
//            for (auto* other : nearby) {
//                if (other == &vehicle) continue;
//                float dist = length(vehicle.getPosition(), other->getPosition());
//                if (dist < 30.f) {
//                    // ??????? ????? – ???? ?????
//                    vehicle.stop(); // ?? ?? ??????? ???
//                    std::cout << "Collision blocked\n";
//                    break; // ?? ????? ????? ?? ????
//                }
//            }
//
//            vehicle.update(dt, blockedPolygons);
//        }
//
//        vehicle.update(dt, blockedPolygons);
//
//        if (vehicle.isInTurn())
//            continue;
//
//        int laneIndex = vehicle.getCurrentLaneIndex();
//        sf::Vector2f endPoint = vehicle.getCurrentRoad()->getLaneEdge(laneIndex, false);
//        float distanceToEnd = length(vehicle.getPosition(), endPoint);
//
//        // ???? ?? ?? ???? ???? ?????
//        if (distanceToEnd > 10.f)
//            continue;
//
//        sf::Vector2f pos = vehicle.getPosition();
//        float queryRange = 160.f;
//        sf::FloatRect queryArea(pos.x - queryRange / 2, pos.y - queryRange / 2, queryRange, queryRange);
//        std::vector<const RoadSegment*> nearbyRoads;
//        roadTree.query(queryArea, nearbyRoads);
//
//        const std::string currentDir = vehicle.getDirection();
//        std::vector<const RoadSegment*> possibleTurns;
//
//        std::cout << "DEBUG: Found " << nearbyRoads.size() << " nearby roads\n";
//        std::random_device rd;
//        std::mt19937 g(rd());
//        std::shuffle(nearbyRoads.begin(), nearbyRoads.end(), g);
//
//
//        for (const RoadSegment* nextRoad : nearbyRoads) {
//            if (nextRoad == vehicle.getCurrentRoad()) {
//                rejectedRoads.emplace_back(nextRoad, "same road");
//                continue;
//            }
//
//            if (nextRoad->bounds.contains(pos)) {
//                std::cout << "not Skipping nextRoad because position still inside its bounds\n";
//                //continue;
//            }
//            int currentLanes = vehicle.getCurrentRoad()->lanes;
//            int nextLanes = nextRoad->lanes;
//          //  int nextLaneIndex = std::min(laneIndex, nextLanes - 1);
//            //int nextLaneIndex = static_cast<int>((float)laneIndex / std::max(1, currentLanes - 1) * (nextLanes - 1));
//            nextLaneIndex = rand() % std::max(0, nextRoad->lanes);
//            std::string actualNextDir = getActualLaneDirection(*nextRoad, nextLaneIndex);
//            std::cout << "currentdir " << currentDir << " nextDie " << actualNextDir << "\n";
//            if (!(isStraight(currentDir, actualNextDir) ||
//                isLeftTurn(currentDir, actualNextDir) ||
//                isRightTurn(currentDir, actualNextDir))) {
//
//                rejectedRoads.emplace_back(nextRoad, "invalid direction from " + currentDir + " to " + actualNextDir);
//                continue;
//            }
//
//            // ????? ?? ????? ???????/?? ?????? ??? ?????
//            //if ((isRightTurn(currentDir, actualNextDir) &&
//            //    (actualNextDir == "right" || actualNextDir == "up"))) {
//            //    rejectedRoads.emplace_back(nextRoad, "invalid right turn into: " + actualNextDir);
//            //    continue;
//            //}
//            //if ((isLeftTurn(currentDir, actualNextDir) &&
//            //    (actualNextDir == "left" || actualNextDir == "down"))) {
//            //    rejectedRoads.emplace_back(nextRoad, "invalid left turn into: " + actualNextDir);
//            //    continue;
//            //}
//
//
//
//            //sf::Vector2f fromEdge = vehicle.getCurrentRoad()->getLaneEdge(nextLaneIndex, false);
//            //sf::Vector2f startEdge = nextRoad->getLaneEdge(laneIndex, true);
//            //sf::Vector2f endEdge = nextRoad->getLaneEdge(laneIndex, false);
//            //sf::Vector2f pos = vehicle.getPosition();
//
//            //bool entryAtStart = (length(pos, startEdge) < length(pos, endEdge));
//            //sf::Vector2f toEdge = nextRoad->getLaneEdge(laneIndex, entryAtStart);
//
//            sf::Vector2f startEdge = nextRoad->getLaneEdge(nextLaneIndex, true);
//            sf::Vector2f endEdge = nextRoad->getLaneEdge(nextLaneIndex, false);
//            sf::Vector2f fromEdge = vehicle.getCurrentRoad()->getLaneEdge(laneIndex, false);
//
//            float distToStart = length(fromEdge, startEdge);
//            float distToEnd = length(fromEdge, endEdge);
//            float minDist = std::min(distToStart, distToEnd);
//
//            std::cout << "[DEBUG] Distance from fromEdge to startEdge: " << distToStart << "\n";
//            std::cout << "[DEBUG] Distance from fromEdge to endEdge: " << distToEnd << "\n";
//
//            bool entryAtStart = (distToStart < distToEnd);
//            sf::Vector2f toEdge = nextRoad->getLaneEdge(nextLaneIndex, entryAtStart);
//
//            sf::Vector2f afterTurnVec;
//            if (actualNextDir == "right") afterTurnVec = { 1.f, 0.f };
//            else if (actualNextDir == "left") afterTurnVec = { -1.f, 0.f };
//            else if (actualNextDir == "up") afterTurnVec = { 0.f, -1.f };
//            else if (actualNextDir == "down") afterTurnVec = { 0.f, 1.f };
//
//            // Move forward ~30 pixels from the landing point of the turn
//            sf::Vector2f forwardPoint = toEdge + afterTurnVec * 30.f;
//
//            // Make sure the car doesn't land at the edge of the road
//            if (!nextRoad->bounds.contains(forwardPoint)) {
//                rejectedRoads.emplace_back(nextRoad, "turn lands too close to end of road");
//                continue;
//            }
//
//
//            // ???? ????? 5 ???????
//            sf::Vector2f dirVec;
//            if (actualNextDir == "right") dirVec = { 1.f, 0.f };
//            else if (actualNextDir == "left") dirVec = { -1.f, 0.f };
//            else if (actualNextDir == "up") dirVec = { 0.f, -1.f };
//            else if (actualNextDir == "down") dirVec = { 0.f, 1.f };
//
//            //sf::Vector2f testStart = toEdge + dirVec * 5.f;
//
//            //float minDist = length(fromEdge, toEdge);
//            std::cout << " from edge : " << fromEdge.x << "," << fromEdge.y  << " to edge " << toEdge.x << "," << toEdge.y << "\n";
//            std::cout << "dist : " << minDist << "\n";
//            if (minDist >= 100.f) {
//                rejectedRoads.emplace_back(nextRoad, "distance too far (" + std::to_string(minDist) + ")");
//                continue;
//            }
//            std::cout << "###################################actual next dir   " << actualNextDir << "lane index " << laneIndex << "\n";
//            
//            if (!isDriveable(*nextRoad, toEdge, actualNextDir, 50.f)) {
//                rejectedRoads.emplace_back(nextRoad, "not driveable");
//                continue;
//            }
//
//
//
//            std::cout << " Valid turn found: From " << currentDir << " to " <<actualNextDir << " | dist=" << minDist << "\n";
//            // === START TURN IMMEDIATELY ===
//            sf::Vector2f from = vehicle.getCurrentRoad()->getLaneEdge(laneIndex, false);
//            sf::Vector2f to = nextRoad->getLaneEdge(nextLaneIndex, entryAtStart);
//
//            std::string turnType = "straight";
//            if (isLeftTurn(currentDir, actualNextDir)) turnType = "left";
//            else if (isRightTurn(currentDir, actualNextDir)) turnType = "right";
//
//            sf::Vector2f control = calcSmartControlPoint(from, to, currentDir, turnType);
//
//            vehicle.startTurn(from, control, to);
//            vehicle.setDirectionVec(actualNextDir);
//            vehicle.setPreviousRoad(vehicle.getCurrentRoad());
//            vehicle.setCurrentRoad(nextRoad);
//            vehicle.setCurrentLaneIndex(nextLaneIndex);
//
//            // ? Exit the loop — no need to check more roads
//            break;
//
//          //  possibleTurns.push_back(nextRoad);
//        }
//
//        std::cout << "DEBUG: Found " << possibleTurns.size() << " possible turns\n";
//
//
//        //if (!possibleTurns.empty()) {
//        //    int chosenIdx = rand() % possibleTurns.size();
//        //    const RoadSegment* chosen = possibleTurns[chosenIdx];
//
//        //    int currentLanes = vehicle.getCurrentRoad()->lanes;
//        //    int nextLanes = chosen->lanes;
//        //    int chosenLaneIndex = nextLaneIndex;
//        //   // if (chosenLaneIndex >= nextLanes) chosenLaneIndex = nextLanes - 1;
//
//        //    sf::Vector2f from = vehicle.getCurrentRoad()->getLaneEdge(laneIndex, false);
//
//        //    std::string actualNextDir = getActualLaneDirection(*chosen, chosenLaneIndex);
//        //    bool entryAtStart = (actualNextDir == "up" || actualNextDir == "left") ? false : true;
//        //    sf::Vector2f to = chosen->getLaneEdge(chosenLaneIndex, entryAtStart);
//
//        //    // ??? ?? ??? ?????? ??? ?????? ????? ?? ?????
//        //    std::string turnType = "straight";
//        //    if (isLeftTurn(currentDir, actualNextDir)) turnType = "left";
//        //    else if (isRightTurn(currentDir, actualNextDir)) turnType = "right";
//
//        //   sf::Vector2f control = calcSmartControlPoint(from, to, currentDir, turnType);
//
//        //    std::cout << "control " << control.x << " " << control.y << "\n";
//
//        //    vehicle.startTurn(from, control, to);
//        //    vehicle.setDirectionVec(actualNextDir);
//        //    vehicle.setPreviousRoad(vehicle.getCurrentRoad());
//        //    vehicle.setCurrentRoad(chosen);
//        //    vehicle.setCurrentLaneIndex(chosenLaneIndex);
//        //}
//
//        //else {
//        //    // ????? U-TURN
//        //    const RoadSegment* currentRoad = vehicle.getCurrentRoad();
//        //    int lanes = currentRoad->lanes;
//        //    int oldLane = vehicle.getCurrentLaneIndex();
//        //    int newLane = lanes - 1 - oldLane;
//
//        //    // ??? ?? ????? ?????? ?????
//        //    std::string currentDir = vehicle.getDirection();
//        //    std::string reverseDir;
//        //    if (currentDir == "up") reverseDir = "down";
//        //    else if (currentDir == "down") reverseDir = "up";
//        //    else if (currentDir == "left") reverseDir = "right";
//        //    else if (currentDir == "right") reverseDir = "left";
//
//        //    // ?? ??? ??????? ?????
//        //    sf::Vector2f dirVec;
//        //    if (currentDir == "up") dirVec = { 0.f, -1.f };
//        //    else if (currentDir == "down") dirVec = { 0.f, 1.f };
//        //    else if (currentDir == "left") dirVec = { -1.f, 0.f };
//        //    else if (currentDir == "right") dirVec = { 1.f, 0.f };
//
//        //    sf::Vector2f from = vehicle.getPosition() - dirVec * 10.f;
//        //    sf::Vector2f to = currentRoad->getLaneEdge(newLane, true); // ???? ?????? ????? ?????
//
//        //    // ????? ????
//        //    sf::Vector2f control = calcSmartControlPoint(from, to, currentDir, "uturn");
//
//        //    vehicle.startTurn(from, control, to);
//        //    vehicle.setDirectionVec(reverseDir);
//        //    vehicle.setCurrentLaneIndex(newLane);
//        //    vehicle.setPreviousRoad(currentRoad);
//        //    vehicle.setCurrentRoad(currentRoad);
//        //}
//
//
//
//    }
//}

void CarManager::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    int nextLaneIndex;
    bool turnFound;

    //   rejectedRoads.clear();

       // ????? vehicleTree ?? ?????? ??????
    vehicleTree.clear();
    for (auto& v : vehicles) {
        sf::Vector2f pos = v.getPosition();
        sf::FloatRect area(pos.x - 0.5f, pos.y - 0.5f, 1.f, 1.f);
        vehicleTree.insert(area, &v);
    }

    for (auto& vehicle : vehicles) {
        // ????? ????????? ?? ????? ??????
        sf::Vector2f pos = vehicle.getPosition();
        sf::FloatRect area(pos.x - 30.f, pos.y - 30.f, 60.f, 60.f);
        std::vector<Vehicle*> nearby;
        vehicleTree.query(area, nearby);

        bool hasCollision = false;
        for (auto* other : nearby) {
            if (other == &vehicle) continue;
            float dist = length(vehicle.getPosition(), other->getPosition());
            if (dist < 30.f) {
                vehicle.stop();
                hasCollision = true;
                //std::cout << "Collision blocked\n";
                break;
            }
        }
        if (hasCollision)
            continue;

        vehicle.update(dt, blockedPolygons);

        if (vehicle.isInTurn())
            continue;

        int laneIndex = vehicle.getCurrentLaneIndex();
        sf::Vector2f endPoint = vehicle.getCurrentRoad()->getLaneEdge(laneIndex, false);
        float distanceToEnd = length(vehicle.getPosition(), endPoint);
        if (distanceToEnd > 10.f)
            continue;

        // ????? ?????? ??????
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

            // ????? ???? ???? ??????
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
            // ????? ???? ?? ?? ????? ????? ????? - ?????? ????? U-turn
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
}


//void CarManager::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
//    int nextLaneIndex;
//    rejectedRoads.clear();
//
//    // ????? vehicleTree ?? ?????? ??????
//    vehicleTree.clear();
//    for (auto& v : vehicles) {
//        sf::Vector2f pos = v.getPosition();
//        sf::FloatRect area(pos.x - 0.5f, pos.y - 0.5f, 1.f, 1.f);
//        vehicleTree.insert(area, &v);
//    }
//
//    for (auto& vehicle : vehicles) {
//        // ????? ????????? ?? ????? ??????
//        sf::Vector2f pos = vehicle.getPosition();
//        sf::FloatRect area(pos.x - 30.f, pos.y - 30.f, 60.f, 60.f);
//        std::vector<Vehicle*> nearby;
//        vehicleTree.query(area, nearby);
//
//        bool hasCollision = false;
//        for (auto* other : nearby) {
//            if (other == &vehicle) continue;
//            float dist = length(vehicle.getPosition(), other->getPosition());
//            if (dist < 30.f) {
//                vehicle.stop();  // ???? ????? ?? ?? ??????? ???
//                hasCollision = true;
//                std::cout << "Collision blocked\n";
//                break;
//            }
//        }
//
//        if (hasCollision)
//            continue;
//
//        vehicle.update(dt, blockedPolygons);
//
//        if (vehicle.isInTurn())
//            continue;
//
//        int laneIndex = vehicle.getCurrentLaneIndex();
//        sf::Vector2f endPoint = vehicle.getCurrentRoad()->getLaneEdge(laneIndex, false);
//        float distanceToEnd = length(vehicle.getPosition(), endPoint);
//        if (distanceToEnd > 10.f)
//            continue;
//
//        // ????? ?????? ??????
//        float queryRange = 160.f;
//        sf::FloatRect queryArea(pos.x - queryRange / 2, pos.y - queryRange / 2, queryRange, queryRange);
//        std::vector<const RoadSegment*> nearbyRoads;
//        roadTree.query(queryArea, nearbyRoads);
//        std::shuffle(nearbyRoads.begin(), nearbyRoads.end(), std::mt19937{ std::random_device{}() });
//
//        const std::string currentDir = vehicle.getDirection();
//
//        bool turnMade = false;
//
//        for (const RoadSegment* nextRoad : nearbyRoads) {
//            if (nextRoad == vehicle.getCurrentRoad()) continue;
//
//            int nextLanes = nextRoad->lanes;
//            nextLaneIndex = rand() % std::max(1, nextLanes);
//
//            std::string actualNextDir = getActualLaneDirection(*nextRoad, nextLaneIndex);
//            if (!(isStraight(currentDir, actualNextDir) ||
//                isLeftTurn(currentDir, actualNextDir) ||
//                isRightTurn(currentDir, actualNextDir))) {
//                continue;
//            }
//
//            sf::Vector2f startEdge = nextRoad->getLaneEdge(nextLaneIndex, true);
//            sf::Vector2f endEdge = nextRoad->getLaneEdge(nextLaneIndex, false);
//            sf::Vector2f fromEdge = vehicle.getCurrentRoad()->getLaneEdge(laneIndex, false);
//
//            float distToStart = length(fromEdge, startEdge);
//            float distToEnd = length(fromEdge, endEdge);
//            bool entryAtStart = distToStart < distToEnd;
//            sf::Vector2f toEdge = nextRoad->getLaneEdge(nextLaneIndex, entryAtStart);
//
//            sf::Vector2f dirVec;
//            if (actualNextDir == "right") dirVec = { 1.f, 0.f };
//            else if (actualNextDir == "left") dirVec = { -1.f, 0.f };
//            else if (actualNextDir == "up") dirVec = { 0.f, -1.f };
//            else if (actualNextDir == "down") dirVec = { 0.f, 1.f };
//
//            sf::Vector2f forwardPoint = toEdge + dirVec * 30.f;
//            if (!nextRoad->bounds.contains(forwardPoint))
//                continue;
//
//            if (!isDriveable(*nextRoad, toEdge, actualNextDir, 50.f))
//                continue;
//
//            std::string turnType = "straight";
//            if (isLeftTurn(currentDir, actualNextDir)) turnType = "left";
//            else if (isRightTurn(currentDir, actualNextDir)) turnType = "right";
//
//            sf::Vector2f control = calcSmartControlPoint(fromEdge, toEdge, currentDir, turnType);
//
//            vehicle.startTurn(fromEdge, control, toEdge);
//            vehicle.setDirectionVec(actualNextDir);
//            vehicle.setPreviousRoad(vehicle.getCurrentRoad());
//            vehicle.setCurrentRoad(nextRoad);
//            vehicle.setCurrentLaneIndex(nextLaneIndex);
//
//            turnMade = true;
//            break; // ??? ??????? ?? ????? ?????
//        }
//
//        if (!turnMade) { // ?? ???? ???? ???? ???? ??? ??? ???? ????? ?? ???? ???? 50 ??????? ?????
//            // U-Turn logic:
//            const RoadSegment* currentRoad = vehicle.getCurrentRoad();
//            int lanes = currentRoad->lanes;
//            int oldLane = vehicle.getCurrentLaneIndex();
//            int newLane = lanes - 1 - oldLane;
//
//            std::string currentDir = vehicle.getDirection();
//            std::string reverseDir;
//            if (currentDir == "up") reverseDir = "down";
//            else if (currentDir == "down") reverseDir = "up";
//            else if (currentDir == "left") reverseDir = "right";
//            else if (currentDir == "right") reverseDir = "left";
//
//            sf::Vector2f dirVec;
//            if (currentDir == "up") dirVec = { 0.f, -1.f };
//            else if (currentDir == "down") dirVec = { 0.f, 1.f };
//            else if (currentDir == "left") dirVec = { -1.f, 0.f };
//            else if (currentDir == "right") dirVec = { 1.f, 0.f };
//
//            sf::Vector2f from = vehicle.getPosition(); //- dirVec * 50.f; // ??? ?????
//            sf::Vector2f to = currentRoad->getLaneEdge(newLane, true); // ????? ????? ?????
//            to = to - dirVec * 50.f;
//
//            // sf::Vector2f control = calcSmartControlPoint(from, to, currentDir, "uturn");
//            sf::Vector2f control = (from + to) * 0.5f;
//            control = control + dirVec * 60.f;
//
//
//
//
//
//            vehicle.startTurn(from, control, to);
//            vehicle.setDirectionVec(reverseDir);
//            vehicle.setCurrentLaneIndex(newLane);
//            vehicle.setPreviousRoad(currentRoad);
//            vehicle.setCurrentRoad(currentRoad);
//        }
//    }
//}



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

    for (auto& vehicle : vehicles) {
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

    Vehicle car;
    car.setTexture(ResourceManager::getInstance().getTexture("car"));
    car.setPosition(laneCenter);
    car.setScale(0.05f, 0.05f);
    std::string actualDir = getActualLaneDirection(road, laneIndex);
    car.setDirectionVec(actualDir);
    car.setCurrentRoad(&road);
    car.setCurrentLaneIndex(laneIndex);

    addVehicle(car);
    vehicleTree.insert(sf::FloatRect(laneCenter.x, laneCenter.y, 1.f, 1.f), &vehicles.back());


   // std::cout << "Spawned car at (" << laneCenter.x << ", " << laneCenter.y
        //<< ") on lane " << laneIndex << " direction: " << actualDir
        //<< " (road#" << roadIdx << ")\n";
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
    // driveableDebugPoints.clear(); // ???? ?????? ?????

    const int steps = 10;
    sf::Vector2f dirVec;
    if (direction == "right")      dirVec = { 1, 0 };
    else if (direction == "left")  dirVec = { -1, 0 };
    else if (direction == "up")    dirVec = { 0, -1 };
    else if (direction == "down")  dirVec = { 0, 1 };
    else {
        //std::cout << "[isDriveable] Invalid direction: " << direction << "\n";
        return false;
    }

    for (int i = 2; i <= steps; ++i) {
        sf::Vector2f point = from + dirVec * (length * (float(i) / steps));
        bool valid = road.bounds.contains(point);
        driveableDebugPoints.push_back({ point, valid });

        if (!valid) {
            /*std::cout << "[isDriveable] FAILED at step " << i
                << " | point: (" << point.x << ", " << point.y << ")"
                << " | direction: " << direction
                << " | road bounds: left=" << road.bounds.left
                << ", top=" << road.bounds.top
                << ", width=" << road.bounds.width
                << ", height=" << road.bounds.height << "\n";*/
            return false;
        }
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
    //min(x) = left
    //max(x) = right
    //min(y) = up
    //max(y) = down
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
