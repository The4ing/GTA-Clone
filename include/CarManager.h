#pragma once
#include "Vehicle.h"
#include "RoadSegment.h"
#include "QuadTree.h"
#include <vector>
#include <SFML/Graphics.hpp>
#include <memory>

class CarManager {
public:
    CarManager();

    // ????? ?????
    void update(float dt);
    void draw(sf::RenderWindow& window);

    std::vector<Vehicle>& getVehicles();
    void addVehicle(const Vehicle& vehicle);

    // ????? ??????
    void loadRoadsFromJSON(const std::string& filename);
    void buildRoadTree();
    std::vector<RoadSegment> findNearbyRoads(const sf::FloatRect& area);
    void spawnSingleVehicleOnRoad();

    // ?????? ?? ????
    std::vector<RoadSegment>& getRoads();
    QuadTree<RoadSegment>& getRoadTree();

private:
    std::vector<Vehicle> vehicles;
    std::vector<RoadSegment> roads;
    QuadTree<RoadSegment> roadTree{ sf::FloatRect(0, 0, 4640, 4672) };

    bool isRightTurn(const std::string& from, const std::string& to);
    bool isLeftTurn(const std::string& from, const std::string& to);
    bool isStraight(const std::string& from, const std::string& to);
    bool isDriveable(const RoadSegment& road, sf::Vector2f from, const std::string& direction, float length);
    sf::Vector2f getEdgePointOfRoad(const RoadSegment& road, const std::string& direction, int laneIndex, bool entry);
    sf::Vector2f calcSmartControlPoint(const sf::Vector2f& from, const sf::Vector2f& to, const std::string& dirFrom, const std::string& dirTo);
    float length(const sf::Vector2f& a, const sf::Vector2f& b);
    std::string getActualLaneDirection(const RoadSegment& road, int laneIndex);
};
