#include "GameManager.h"
#include <iostream>
#include "ResourceManager.h"
#include "Slideshow.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
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

    if (carManager)
        carManager->update(dt, blockedPolygons);

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
    chunkManager->updateObjects(dt, blockedPolygons);
    if (policeManager)
        policeManager->update(dt, player->getPosition(), blockedPolygons);

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

    if (carManager)
        carManager->draw(window);
    if (policeManager)
        policeManager->draw(window);

    window.display();
}

void GameManager::startGameFullscreen() {
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    window.create(desktop, "Top-Down GTA Clone", sf::Style::Fullscreen);
    window.setFramerateLimit(60);

    //for checking
    policeManager = std::make_unique<PoliceManager>();
    policeManager->spawnPolice({ 100.f, 100.f }); 

    carManager = std::make_unique<CarManager>();
    carManager->loadRoadsFromJSON("resources/try.tmj");
    carManager->buildRoadTree();
    carManager->spawnSingleVehicleOnRoad();
    carManager->spawnSingleVehicleOnRoad();
    carManager->spawnSingleVehicleOnRoad();

    chunkManager = std::make_unique<ChunkManager>();
    player = std::make_unique<Player>();
    player->setPosition({ 100.f, 100.f });

    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);
    gameView.setSize(winW, winH);
    gameView.zoom(0.25f);

    currentState = GameState::Playing;
}
