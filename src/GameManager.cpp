#include "GameManager.h"
#include <iostream>
#include "ResourceManager.h"
#include "Slideshow.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <cstdlib>
#include <ctime>
using json = nlohmann::json;

GameManager::GameManager()
    : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Top-Down GTA Clone") {
    window.setFramerateLimit(60);
    menu = std::make_unique<Menu>(window);
    currentState = GameState::Menu;
}

void GameManager::run() {
    Slideshow slideshow(window, 2.5f);
    //slideshow.run();

    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        processEvents();

        if (currentState == GameState::Menu) {
            if (menu->isOptionChosen()) {
                std::string selected = menu->getSelectedOption();
                if (selected == "Start Game")
                    startGameFullscreen();
                else if (selected == "Exit")
                    window.close();
            }

            window.clear();
            menu->draw();
            window.display();

        }
        else if (currentState == GameState::Playing) {
            update(deltaTime);
            render();
        }
    }
}

void GameManager::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();

        if (currentState == GameState::Menu) {
            menu->update(event);
        }
    }
}

void GameManager::update(float dt) {
    player->update(dt, blockedPolygons);

    for (auto& vehicle : vehicles) {
        vehicle.update(dt);

        if (!vehicle.isInTurn()) {
            sf::Vector2f pos = vehicle.getPosition();
            float queryRange = 100.f;

            sf::FloatRect queryArea(pos.x - queryRange / 2, pos.y - queryRange / 2, queryRange, queryRange);

            std::vector<const RoadSegment*> nearbyRoads;
            roadTree.query(queryArea, nearbyRoads);  // עכשיו זה בטוח ולא נגרם heap-use-after-free!


            const std::string currentDir = vehicle.getDirection();  

            std::vector<const RoadSegment*> possibleTurns;

            for (const RoadSegment* nextRoad : nearbyRoads) {
                if (!nextRoad->bounds.contains(pos)) {
                    std::string nextDir = nextRoad->direction;

                    if (isStraight(currentDir, nextDir) ||
                        isLeftTurn(currentDir, nextDir) ||
                        isRightTurn(currentDir, nextDir)) {

                        int laneIndex = vehicle.getCurrentLaneIndex();
                        // Exit point of current road:
                        sf::Vector2f fromEdge = getEdgePointOfRoad(*vehicle.getCurrentRoad(), currentDir, laneIndex, false);
                        // Entry point of next road:
                        sf::Vector2f toEdge = getEdgePointOfRoad(*nextRoad, nextDir, laneIndex, true);
                        float dist = length(fromEdge, toEdge);

                        std::cout << "Checking transition from " << currentDir << " to " << nextDir
                            << " | Edge distance: " << dist << std::endl;

                        if (dist < 50.f && isDriveable(*nextRoad, toEdge, nextDir, 50.0f)) {
                            std::cout << "---- Added possible turn to " << nextDir << std::endl;
                            possibleTurns.push_back(nextRoad);
                        }

                        std::cout << "Total possible turns: " << possibleTurns.size() << std::endl;


                    }
                }
            }

            if (!possibleTurns.empty()) {
                const RoadSegment* chosen = possibleTurns[0];

                int currentLanes = vehicle.getCurrentRoad()->lanes;
                int nextLanes = chosen->lanes;
                int laneIndex = vehicle.getCurrentLaneIndex();
                if (laneIndex >= nextLanes) laneIndex = nextLanes - 1;

                sf::Vector2f from = vehicle.getCurrentRoad()->getLaneEdge(laneIndex, false); // false = יציאה
                sf::Vector2f to = chosen->getLaneEdge(laneIndex, true);                    // true  = כניסה

                // חישוב נקודת שליטה (Bezier Control)
                sf::Vector2f mid = (from + to) / 2.f;
                sf::Vector2f dir = to - from;
                float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (len == 0) len = 1;
                dir /= len;

                bool leftTurn = isLeftTurn(currentDir, chosen->direction);
                bool rightTurn = isRightTurn(currentDir, chosen->direction);

                sf::Vector2f perp(0, 0);
                float turnDepth = 80.f;
                if (leftTurn)      perp = sf::Vector2f(-dir.y, dir.x);
                else if (rightTurn)perp = sf::Vector2f(dir.y, -dir.x);

                sf::Vector2f control = mid + perp * turnDepth;

                vehicle.startTurn(from, control, to);
                vehicle.setDirectionVec(getActualLaneDirection(*chosen, laneIndex));
                vehicle.setCurrentRoad(chosen);
                vehicle.setCurrentLaneIndex(laneIndex); // <- שים לב!
            }



        }
    }


    // עדכון תצוגה
    sf::Vector2f playerPos = player->getPosition();
    sf::Vector2f newCenter = playerPos;

    sf::Vector2f viewSize = gameView.getSize();
    float halfW = viewSize.x * 0.5f;
    float halfH = viewSize.y * 0.5f;

    if (viewSize.x > MAP_WIDTH)  halfW = MAP_WIDTH * 0.5f;
    if (viewSize.y > MAP_HEIGHT) halfH = MAP_HEIGHT * 0.5f;

    if (newCenter.x < halfW)               newCenter.x = halfW;
    if (newCenter.x > (MAP_WIDTH - halfW)) newCenter.x = MAP_WIDTH - halfW;
    if (newCenter.y < halfH)               newCenter.y = halfH;
    if (newCenter.y > (MAP_HEIGHT - halfH))newCenter.y = MAP_HEIGHT - halfH;

    gameView.setCenter(newCenter);
    chunkManager->updateChunks(newCenter, gameView);
    chunkManager->updateObjects(dt);
}


void GameManager::render() {
    window.clear(sf::Color::Black);
    window.setView(gameView);
    chunkManager->draw(window, gameView);
    player->draw(window);

    for (const auto& poly : blockedPolygons) {
        sf::ConvexShape shape;
        shape.setPointCount(poly.size());
        for (size_t i = 0; i < poly.size(); ++i)
            shape.setPoint(i, poly[i]);
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineColor(sf::Color::Green);
        shape.setOutlineThickness(1.f);
        window.draw(shape);
    }
    // ציור רכבים
    for (auto& vehicle : vehicles)
        vehicle.draw(window);

    window.display();
}

void GameManager::startGameFullscreen() {
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    window.create(desktop, "Top-Down GTA Clone", sf::Style::Fullscreen);
    window.setFramerateLimit(60);

    loadCollisionRectsFromJSON("resources/try.tmj");

    // בנה את העץ לאחר טעינת הכבישים
    buildRoadTree();

    // צור רכב לדוגמה
    spawnSingleVehicleOnRoad();

    chunkManager = std::make_unique<ChunkManager>();
    player = std::make_unique<Player>();
    player->setPosition({ 100.f, 100.f });

    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);
    gameView.setSize(winW, winH);
    gameView.zoom(0.25f);

    currentState = GameState::Playing;
}


void GameManager::loadCollisionRectsFromJSON(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open " << filename << std::endl;
        return;
    }

    json data;
    file >> data;

    for (const auto& layer : data["layers"]) {
        if (layer["type"] == "objectgroup" && (layer["name"] == "collision" || layer["name"] == "roads")) {
            for (const auto& obj : layer["objects"]) {
                float x = obj["x"];
                float y = obj["y"];

                // Rectangles as polygons
                if (obj.contains("width") && obj.contains("height")) {
                    float w = obj["width"];
                    float h = obj["height"];
                    std::vector<sf::Vector2f> polygon;
                    polygon.emplace_back(x, y);
                    polygon.emplace_back(x + w, y);
                    polygon.emplace_back(x + w, y + h);
                    polygon.emplace_back(x, y + h);
                    if (layer["name"] != "roads")  // רק אם זה לא כביש, נכניס את זה כחסם
                        blockedPolygons.push_back(polygon);
                }

                // Polygons
                if (obj.contains("polygon")) {
                    std::vector<sf::Vector2f> polygon;
                    for (const auto& point : obj["polygon"]) {
                        float px = x + point["x"].get<float>();
                        float py = y + point["y"].get<float>();
                        polygon.emplace_back(px, py);
                    }
                    blockedPolygons.push_back(polygon);
                }

                // 🚗 Road segment detection inside collision layer
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

                    // We only add if it has a direction
                    if (!road.direction.empty())
                        roads.push_back(road);
                }
            }
        }
    }

    std::cout << "Loaded " << blockedPolygons.size() << " polygons\n";
    std::cout << "Loaded " << roads.size() << " road segments\n";
}

void GameManager::spawnSingleVehicleOnRoad() {
    if (roads.empty()) {
        std::cerr << "No roads loaded to place vehicle.\n";
        return;
    }

    // בחר קטע כביש אקראי
    //const RoadSegment& road = roads[rand() % roads.size()];
    const RoadSegment&  road = roads[0];

    // בחר נתיב אקראי
    int laneIndex = rand() % std::max(1, road.lanes);
    laneIndex = 3;

    sf::Vector2f laneCenter = road.getLaneCenter(laneIndex);

    // אורך רכב משוער
    float carLength = 50.f;

    // הוסף מיקום רנדומלי לאורך הנתיב
    if (road.direction == "up" || road.direction == "down") {
        float maxOffset = road.bounds.height - carLength;
        if (maxOffset < 0) maxOffset = 0;

        float offset = static_cast<float>(rand()) / RAND_MAX * maxOffset;

        if (road.direction == "up")
            laneCenter.y = road.bounds.top + road.bounds.height - offset;
        else
            laneCenter.y = road.bounds.top + offset;

        laneCenter.y = std::clamp(laneCenter.y, road.bounds.top, road.bounds.top + road.bounds.height);
    }
    else if (road.direction == "left" || road.direction == "right") {
        float maxOffset = road.bounds.width - carLength;
        if (maxOffset < 0) maxOffset = 0;

        float offset = static_cast<float>(rand()) / RAND_MAX * maxOffset;

        if (road.direction == "left")
            laneCenter.x = road.bounds.left + road.bounds.width - offset;
        else
            laneCenter.x = road.bounds.left + offset;

        laneCenter.x = std::clamp(laneCenter.x, road.bounds.left, road.bounds.left + road.bounds.width);
    }

    // צור את הרכב
    Vehicle car;
    car.setTexture(ResourceManager::getInstance().getTexture("car"));  // ודא שטקסטורה בשם הזה קיימת
    car.setPosition(laneCenter);
    std::string actualDir = getActualLaneDirection(road, laneIndex);
    car.setDirectionVec(actualDir);
    car.setScale(0.05f, 0.05f);  // שים סקייל לפי גודל הרכב הרצוי
    car.setCurrentRoad(&road); // חובה כדי למנוע לולאות פנייה

    car.setCurrentLaneIndex(laneIndex);
    vehicles.push_back(car);

    std::cout << "Spawned car at (" << laneCenter.x << ", " << laneCenter.y << ") on lane "
        << laneIndex << " direction: " << actualDir << "\n";
}



std::string GameManager::getActualLaneDirection(const RoadSegment& road, int laneIndex) {
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

void GameManager::buildRoadTree() {
    roadTree = QuadTree<RoadSegment>(sf::FloatRect(0, 0, 4640, 4672));
    for (const auto& road : roads) {
        roadTree.insert(road.bounds, road);
    }
}

std::vector<RoadSegment> GameManager::findNearbyRoads(const sf::FloatRect& area) {
    return roadTree.query(area);
}



bool GameManager::isRightTurn(const std::string& from, const std::string& to) {
    return (from == "up" && to == "right") ||
        (from == "right" && to == "down") ||
        (from == "down" && to == "left") ||
        (from == "left" && to == "up");
}

bool GameManager::isLeftTurn(const std::string& from, const std::string& to) {
    return (from == "up" && to == "left") ||
        (from == "left" && to == "down") ||
        (from == "down" && to == "right") ||
        (from == "right" && to == "up");
}

bool GameManager::isStraight(const std::string& from, const std::string& to) {
    return (from == to);
}

bool GameManager::isDriveable(const RoadSegment& road, sf::Vector2f from, const std::string& direction, float length) {
    const int steps = 10; // אפשר להגדיל ליותר צעדים לרזולוציה גבוהה יותר
    sf::Vector2f dirVec;
    if (direction == "right")      dirVec = { 1, 0 };
    else if (direction == "left")  dirVec = { -1, 0 };
    else if (direction == "up")    dirVec = { 0, -1 };
    else if (direction == "down")  dirVec = { 0, 1 };
    else return false;

    for (int i = 1; i <= steps; ++i) {
        sf::Vector2f point = from + dirVec * (length * (float(i) / steps));
        if (!road.bounds.contains(point))
            return false; // אם יצאנו מהכביש – הכביש קצר מדי!
    }
    return true;
}

sf::Vector2f GameManager::getEdgePointOfRoad(const RoadSegment& road, const std::string& direction, int laneIndex, bool entry) {
    // If entry==true, return the entry point for this direction
    // If entry==false, return the exit point for this direction
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
    // Default:
    return { road.bounds.left, road.bounds.top };
}

float GameManager::length(const sf::Vector2f& a, const sf::Vector2f& b) {
    return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}
