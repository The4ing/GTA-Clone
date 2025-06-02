#include "GameManager.h"
#include <iostream>
#include "ResourceManager.h"
#include "Slideshow.h"
#include <nlohmann/json.hpp>
#include <fstream>
using json = nlohmann::json;


GameManager::GameManager()
    : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Top-Down GTA Clone") {
    window.setFramerateLimit(60);

    menu = std::make_unique<Menu>(window);
    currentState = GameState::Menu;
}

void GameManager::run() {

    Slideshow slideshow(window, 2.5f);
    slideshow.run();

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


// GameManager.cpp

void GameManager::update(float dt) {
    player->update(dt, blockedAreas); 
        sf::Vector2f playerPos = player->getPosition();
    sf::Vector2f newCenter = playerPos;

    sf::Vector2f viewSize = gameView.getSize(); 
        float halfW = viewSize.x * 0.5f;
    float halfH = viewSize.y * 0.5f;

    if (newCenter.x < halfW)              newCenter.x = halfW;
    if (newCenter.x > (MAP_WIDTH - halfW)) newCenter.x = MAP_WIDTH - halfW;
    if (newCenter.y < halfH)              newCenter.y = halfH;
    if (newCenter.y > (MAP_HEIGHT - halfH))newCenter.y = MAP_HEIGHT - halfH;
  
    gameView.setCenter(newCenter); 
    chunkManager->updateChunks(newCenter, gameView);

   
}


// GameManager.cpp

void GameManager::render() {
    window.clear(sf::Color::Black);

    
    window.setView(gameView);

    
    chunkManager->draw(window, gameView);

    
    player->draw(window);

    

    window.display();
}


void GameManager::startGameFullscreen() {

    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();

    window.create(desktop, "Top-Down GTA Clone", sf::Style::Fullscreen);
    window.setFramerateLimit(60);

    loadCollisionRectsFromJSON("resources/test.tmj");

    
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
        std::cerr << "CAnt open " << filename << std::endl;
        return;
    }

    json data;
    file >> data;

    for (const auto& layer : data["layers"]) {
        if (layer["type"] == "objectgroup" && layer["name"] == "collision") {
            for (const auto& obj : layer["objects"]) {
                float x = obj["x"];
                float y = obj["y"];
                float w = obj["width"];
                float h = obj["height"];
                blockedAreas.emplace_back(x, y, w, h);
            }
        }
    }

    std::cout << "there was  " << blockedAreas.size() << " area Loded\n";
}