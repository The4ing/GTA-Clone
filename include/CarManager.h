#pragma once
#include "Vehicle.h"
#include "RoadSegment.h"
#include "QuadTree.h"
#include <vector>
#include <SFML/Graphics.hpp>
#include <memory>
#include <iostream>

class CarManager {
public:
    CarManager();

    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void draw(sf::RenderWindow& window);

    std::vector<Vehicle>& getVehicles();
    void addVehicle(const Vehicle& vehicle);

    void setRoads(const std::vector<RoadSegment>& newRoads);
    void buildRoadTree();
    std::vector<RoadSegment> findNearbyRoads(const sf::FloatRect& area);
    void spawnSingleVehicleOnRoad();

    std::vector<RoadSegment>& getRoads();
    QuadTree<RoadSegment>& getRoadTree();


private:
    sf::Vector2f forwardPoint; // debug
    struct DebugPoint {
        sf::Vector2f pos;
        bool valid;
    };
    std::vector<DebugPoint> driveableDebugPoints;


    std::vector<std::pair<const RoadSegment*, std::string>> rejectedRoads;  // debug

    std::vector<Vehicle> vehicles;
    QuadTree<Vehicle*> vehicleTree;

    std::vector<RoadSegment> roads;
    QuadTree<RoadSegment> roadTree{ sf::FloatRect(0, 0, 4640, 4672) };
    sf::Font debugFont;

    bool isRightTurn(const std::string& from, const std::string& to);
    bool isLeftTurn(const std::string& from, const std::string& to);
    bool isStraight(const std::string& from, const std::string& to);
    bool isDriveable(const RoadSegment& road, sf::Vector2f from, const std::string& direction, float length);
    float length(const sf::Vector2f& a, const sf::Vector2f& b);
    sf::Vector2f calcSmartControlPoint(const sf::Vector2f& from, const sf::Vector2f& to, const std::string& dirFrom, const std::string& turnType);
    std::string getActualLaneDirection(const RoadSegment& road, int laneIndex);
};
