#include "GameManager.h"
#include <iostream>
#include "ResourceManager.h"
#include "Slideshow.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include "GameFactory.h"

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

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F11) {
            setFullscreen(!isFullscreen); 
        }

        if (currentState == GameState::Menu) {
            menu->update(event);
        }
    }
}


void GameManager::update(float dt) {
    player->update(dt, blockedPolygons);

    if (carManager)
        carManager->update(dt, blockedPolygons);

    
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
   //chunkManager->updateChunks(newCenter, gameView);
 //   chunkManager->updateObjects(dt, blockedPolygons);

    if (policeManager) 
        policeManager->update(dt, player->getPosition(), blockedPolygons);
    
    if (pedestrianManager)
        pedestrianManager->update(dt, blockedPolygons);

    for (auto& present : presents)
        present->update(dt, blockedPolygons);


    for (auto& present : presents) {
        if (!present->isCollected()) {
            if (player->getCollisionBounds().intersects(present->getSprite().getGlobalBounds())) {
                player->onCollision(*present);  // Double Dispatch
            }
        }
    }


}

void GameManager::render() {
    window.clear(sf::Color::Black);
    window.setView(gameView);
    window.draw(mapSprite);

 //   chunkManager->draw(window, gameView);
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

    if (carManager)
        carManager->draw(window);
    if (policeManager)
        policeManager->draw(window);
    if (pedestrianManager)
        pedestrianManager->draw(window);
    for (auto& present : presents)
    present->draw(window);

    window.display();
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
                    if (layer["name"] != "roads")
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

                // Road segment detection inside collision layer
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

    std::cout << "Loaded " << blockedPolygons.size() << " polygons\n";
    std::cout << "Loaded " << roads.size() << " road segments\n";
}


void GameManager::buildBlockedPolyTree() {
    blockedPolyTree = QuadTree<std::vector<sf::Vector2f>>(sf::FloatRect(0, 0, 4640, 4672));
    for (const auto& poly : blockedPolygons) {
        float minX = poly[0].x, minY = poly[0].y, maxX = poly[0].x, maxY = poly[0].y;
        for (const auto& pt : poly) {
            minX = std::min(minX, pt.x);
            minY = std::min(minY, pt.y);
            maxX = std::max(maxX, pt.x);
            maxY = std::max(maxY, pt.y);
        }
        blockedPolyTree.insert(sf::FloatRect(minX, minY, maxX - minX, maxY - minY), poly);
    }

}



void GameManager::startGameFullscreen() {
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    window.create(desktop, "Top-Down GTA Clone", sf::Style::Fullscreen);
    window.setFramerateLimit(60);
    isFullscreen = true;

    gameView.setSize(static_cast<float>(desktop.width), static_cast<float>(desktop.height));
    gameView.setCenter(gameView.getSize().x / 2.f, gameView.getSize().y / 2.f);
    window.setView(gameView);


    loadCollisionRectsFromJSON("resources/map.tmj");
    buildBlockedPolyTree();
    mapTexture = &ResourceManager::getInstance().getTexture("map");
    mapSprite.setTexture(*mapTexture);
    //mapSprite.setPosition(0, 0);


//    pedestrianManager = GameFactory::createPedestrianManager();
  //  policeManager = GameFactory::createPoliceManager();
    pedestrianManager = GameFactory::createPedestrianManager(blockedPolygons);
    policeManager = GameFactory::createPoliceManager(blockedPolygons);


    carManager = GameFactory::createCarManager(roads);

    //    for (int i = 0; i < 20; ++i) {
    carManager->spawnSingleVehicleOnRoad();
    //  }
    //  carManager->spawnSingleVehicleOnRoad();
     // carManager->spawnSingleVehicleOnRoad();

    //  chunkManager = GameFactory::createChunkManager();
    player = GameFactory::createPlayer({ 100.f, 100.f });
    //presents = GameFactory::createPresents(30, blockedPolygons);
    presents = GameFactory::createPresents(1, blockedPolygons);

    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);
    gameView.setSize(winW, winH);
    gameView.zoom(0.25f);

    currentState = GameState::Playing;
}

void GameManager::setFullscreen(bool fullscreen) {
    if (fullscreen == isFullscreen)
        return;

    isFullscreen = fullscreen;

    sf::Vector2u size;
    sf::Uint32 style;

    if (fullscreen) {
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        size = { desktop.width, desktop.height };
        style = sf::Style::Fullscreen;
    }
    else {
        size = { WINDOW_WIDTH, WINDOW_HEIGHT };
        style = sf::Style::Default;
    }

    window.create(sf::VideoMode(size.x, size.y), "Top-Down GTA Clone", style);
    window.setFramerateLimit(60);

    // ????? ???? ?-View ?????? ????? ????:
    gameView.setSize(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT));
    gameView.setCenter(gameView.getSize().x / 2.f, gameView.getSize().y / 2.f);
    window.setView(gameView);

    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);
    gameView.setSize(winW, winH);
    gameView.zoom(0.25f);


}
