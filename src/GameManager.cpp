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
    // slideshow.run();

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
            std::vector<RoadSegment> foundRoads = roadTree.query(queryArea); // שלב 1 – וקטור של אובייקטים
            std::vector<const RoadSegment*> nearbyRoads;                     // שלב 2 – וקטור של פוינטרים

            for (const RoadSegment& road : foundRoads)
                nearbyRoads.push_back(&road);

            const std::string currentDir = vehicle.getDirection();  // נניח שמימשת getter

            std::vector<const RoadSegment*> possibleTurns;

            for (const RoadSegment* nextRoad : nearbyRoads) {
                if (!nextRoad->bounds.contains(pos)) {
                    std::string nextDir = nextRoad->direction;

                    if (isStraight(currentDir, nextDir) ||
                        isLeftTurn(currentDir, nextDir) ||
                        isRightTurn(currentDir, nextDir)) {

                        possibleTurns.push_back(nextRoad);
                    }
                }
            }

            if (!possibleTurns.empty()) {
                // תבחר כביש באקראי מתוך האפשרויות
                const RoadSegment* chosen = possibleTurns[rand() % possibleTurns.size()];

                sf::Vector2f from = pos;
                sf::Vector2f to = chosen->getLaneCenter(0);
                sf::Vector2f control = (from + to) / 2.f;

                vehicle.startTurn(from, control, to);
                vehicle.setDirectionVec(getActualLaneDirection(*chosen, 0));
                vehicle.setCurrentRoad(chosen);
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
    const RoadSegment& road = roads[rand() % roads.size()];

    // בחר נתיב אקראי
    int laneIndex = rand() % std::max(1, road.lanes);
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


    vehicles.push_back(car);

    std::cout << "Spawned car at (" << laneCenter.x << ", " << laneCenter.y << ") on lane "
        << laneIndex << " direction: " << road.direction << "\n";
}



std::string GameManager::getActualLaneDirection(const RoadSegment& road, int laneIndex) {
    if (!road.is2D) return road.direction;

    int half = road.lanes / 2;
    cout << "direction :" << road.direction << " and laneindex: " << laneIndex << " and half:  " << half;
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

void GameManager::handleVehicleTurning(Vehicle& car) {
    if (!car.currentRoad) return;

    const RoadSegment* road = car.currentRoad;
    sf::Vector2f pos = car.getPosition();

    // הגדר רדיוס חיפוש סביר סביב הרכב
    float queryRadius = 30.f;
    sf::FloatRect searchArea(pos.x - queryRadius, pos.y - queryRadius, queryRadius * 2, queryRadius * 2);

    // חיפוש בצומת
    std::vector<RoadSegment*> nearby;
    roadTree.query(searchArea, nearby);

    for (RoadSegment* nextRoad : nearby) {
        if (nextRoad == road) continue;

        // נבדוק אם הוא ממש נוגע במיקום הרכב (כלומר צומת בפועל)
        if (nextRoad->bounds.contains(pos)) {
            // כאן אפשר להוסיף לוגיקת החלטה - ישר / פנייה
            std::string newDir = getActualLaneDirection(*nextRoad, 0);

            // התחלת תמרון (Bezier)
            sf::Vector2f from = car.getPosition();
            sf::Vector2f to = nextRoad->getLaneCenter(0);
            sf::Vector2f control = (from + to) * 0.5f;  // נקודת אמצע לבקרת העקומה

            car.startTurn(from, control, to);
            car.setDirectionVec(newDir);
            car.setCurrentRoad(nextRoad);

            return;
        }
    }
}

bool GameManager::shouldTurnTo(const Vehicle& vehicle, const RoadSegment& candidateRoad) {
    sf::Vector2f pos = vehicle.getPosition();
    sf::FloatRect bounds = candidateRoad.bounds;

    const float threshold = 40.f;  // רדיוס סביר לגבול
    bool closeToEdge =
        std::abs(pos.x - bounds.left) < threshold ||
        std::abs(pos.x - (bounds.left + bounds.width)) < threshold ||
        std::abs(pos.y - bounds.top) < threshold ||
        std::abs(pos.y - (bounds.top + bounds.height)) < threshold;

    if (!closeToEdge)
        return false;

    // כיוון נוכחי של הרכב
    std::string currentDir = vehicle.getDirection();
    std::string nextDir = candidateRoad.direction;

    // לבדוק אם הפנייה הגיונית (לא סיבוב של 180 מעלות)
    if (currentDir == "up" && nextDir == "down") return false;
    if (currentDir == "down" && nextDir == "up")   return false;
    if (currentDir == "left" && nextDir == "right")return false;
    if (currentDir == "right" && nextDir == "left") return false;

    return true;
}

bool GameManager::isNearEdge(const Vehicle& v, const RoadSegment& road) {
    const float threshold = 30.f;
    sf::Vector2f pos = v.getPosition();
    return std::abs(pos.x - road.bounds.left) < threshold ||
        std::abs(pos.x - (road.bounds.left + road.bounds.width)) < threshold ||
        std::abs(pos.y - road.bounds.top) < threshold ||
        std::abs(pos.y - (road.bounds.top + road.bounds.height)) < threshold;
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
