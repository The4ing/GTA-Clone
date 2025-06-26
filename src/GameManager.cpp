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
#include "Vehicle.h"
#include <limits>
#include "player.h"
#include "InventoryUI.h"

using json = nlohmann::json;

// Helper function to calculate distance between two points (squared, to avoid sqrt initially)Add commentMore actions
float distanceSquared(const sf::Vector2f& p1, const sf::Vector2f& p2) {
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    return dx * dx + dy * dy;
}

GameManager::GameManager()
    : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Top-Down GTA Clone"), m_gameTime(sf::Time::Zero)
{
    frozenBackgroundTexture.create(window.getSize().x, window.getSize().y);
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
        //
        else if (currentState == GameState::Inventory) {
            //inventoryUI.handleInput(*player ,player->getInventory(), window);
           
            window.clear();
           // inventoryUI.draw(window, player->getInventory());
            window.setView(window.getDefaultView());  // הצגה לפי View רגיל של המסך
            inventoryUI.handleInput(*player, player->getInventory(), window);
            window.draw(frozenBackgroundSprite); // הרקע הקפואAdd commentMore actions
            inventoryUI.draw(window, player->getInventory()); // האינבנטורי מעל
            window.display();
        }

    }
}

void GameManager::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
        if (event.type == sf::Event::Resized) {
            float w = static_cast<float>(event.size.width);
            float h = static_cast<float>(event.size.height);
            if (m_hud) {
                m_hud->updateElementPositions(w, h);
            }
        }

        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::F11)
                setFullscreen(!isFullscreen);
            if (currentState == GameState::Playing && event.key.code == sf::Keyboard::I) {
                // שמור את המצב הנוכחי של המשחק לתמונהAdd commentMore actions
                frozenBackgroundTexture.clear();
                frozenBackgroundTexture.setView(gameView); // חשוב מאוד!
                renderFrozenGame(frozenBackgroundTexture);
                frozenBackgroundTexture.display();
                frozenBackgroundSprite.setTexture(frozenBackgroundTexture.getTexture());
                frozenBackgroundSprite.setPosition(0, 0);
                frozenBackgroundSprite.setScale(
                    float(window.getSize().x) / frozenBackgroundTexture.getSize().x,
                    float(window.getSize().y) / frozenBackgroundTexture.getSize().y
                );
                currentState = GameState::Inventory;
            }
            else if (currentState == GameState::Inventory && event.key.code == sf::Keyboard::Escape) {
                currentState = GameState::Playing;
            }
            // Enter/Exit Vehicle LogicAdd commentMore actionsAdd commentMore actions
            if (currentState == GameState::Playing && event.key.code == sf::Keyboard::F) {
                if (player && carManager) { // Ensure player and carManager exist
                    if (player->isInVehicle()) {
                        // Player is in a vehicle, so exit
                        Vehicle* currentVehicle = player->getCurrentVehicle();
                        player->exitVehicle(); // Player handles its state change
                        if (currentVehicle) {
                            currentVehicle->setDriver(nullptr); // Vehicle no longer has a driver
                        }
                    }
                    else {
                        // Player is on foot, try to enter a nearby vehicle
                        Vehicle* closestVehicle = nullptr;
                        float minDistanceSq = std::numeric_limits<float>::max();
                        const float enterRadius = 75.f; // Max distance to enter a vehicle
                        const float enterRadiusSq = enterRadius * enterRadius;

                        sf::Vector2f playerPos = player->getPosition();

                        for (auto& vehicle : carManager->getVehicles()) {
                            if (!vehicle.hasDriver()) { // Check if vehicle is empty
                                float distSq = distanceSquared(playerPos, vehicle.getPosition());
                                if (distSq < enterRadiusSq && distSq < minDistanceSq) {
                                    minDistanceSq = distSq;
                                    closestVehicle = &vehicle;
                                }
                            }
                        }

                        if (closestVehicle) {
                            player->enterVehicle(closestVehicle);
                            closestVehicle->setDriver(player.get());
                        }
                    }
                }
            }
        }

        if (currentState == GameState::Menu) {
            menu->update(event);
        }
    }
}


void GameManager::renderFrozenGame(sf::RenderTarget& target) {
    target.draw(mapSprite);
    player->draw(target);

    for (const auto& poly : blockedPolygons) {
        sf::ConvexShape shape;
        shape.setPointCount(poly.size());
        for (size_t i = 0; i < poly.size(); ++i)
            shape.setPoint(i, poly[i]);
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineColor(sf::Color::Green);
        shape.setOutlineThickness(1.f);
        target.draw(shape);
    }

    if (carManager)
        carManager->draw(target);
    if (policeManager)
        policeManager->draw(target);
    if (pedestrianManager)
        pedestrianManager->draw(target);
    for (auto& present : presents)
        present->draw(target);
    for (const auto& bullet_ptr : bulletPool.getAllBullets()) { // Draw bullets from pool
        if (bullet_ptr->isActive()) {
            bullet_ptr->draw(target);
        }
    }
}

void GameManager::update(float dt) {
    // Update game time
    m_gameTime += sf::seconds(dt * GAME_TIME_SCALE);

    if (!player) return; // Early exit if player isn't initialized
    player->update(dt, blockedPolygons);

    if (carManager)
        carManager->update(dt, blockedPolygons);


    //  sf::Vector2f playerPos = player->getPosition();Add commentMore actions
  //  sf::Vector2f newCenter = playerPos;

    sf::Vector2f focusPosition;
    if (player->isInVehicle() && player->getCurrentVehicle()) {
        focusPosition = player->getCurrentVehicle()->getPosition();
    }
    else {
        focusPosition = player->getPosition();
    }
    sf::Vector2f newCenter = focusPosition;
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

    if (policeManager)
        policeManager->update(dt, player->getPosition(), blockedPolygons);

    if (pedestrianManager)
        pedestrianManager->update(dt, blockedPolygons);

    for (auto& present : presents)
        present->update(dt, blockedPolygons);

    // Update bullets from the pool
    for (auto& bullet_ptr : bulletPool.getAllBullets()) {
        if (bullet_ptr->isActive()) {
            bullet_ptr->update(dt, blockedPolygons);

            std::vector<Pedestrian> current_npcs;
            if (pedestrianManager) {
                for (const auto& p_ptr : pedestrianManager->getPedestrians()) {
                    if (p_ptr) current_npcs.push_back(*p_ptr);
                }
            }
            std::vector<Vehicle> current_cars;
            if (carManager) {
                current_cars = carManager->getVehicles();
            }

            if (bullet_ptr->checkCollision(blockedPolygons, current_npcs, current_cars)) {
                // checkCollision now sets active to false. BulletPool::returnBullet is not strictly needed here
                // as the bullet is already marked inactive. The pool reuses inactive bullets.
                // However, if returnBullet had other logic (e.g. tracking), it would be called.
                // For now, checkCollision handles making it inactive.
            }
        }
    }

    // No need to explicitly remove bullets from a vector here,
    // as the pool manages inactive bullets.

    for (auto& present : presents) {
        if (!present->isCollected()) {
            if (player->getCollisionBounds().intersects(present->getSprite().getGlobalBounds())) {
                player->onCollision(*present);  // Double Dispatch
            }
        }
    }
    // Update HUD
    if (m_hud && player && currentState == GameState::Playing) {
        PlayerData playerData;
        playerData.money = player->getMoney();
        playerData.health = player->getHealth();
        playerData.armor = player->getArmor();
        playerData.weaponName = player->getCurrentWeaponName();
        //playerData.currentAmmo = player->getCurrentAmmo();
        playerData.maxAmmo = player->getMaxAmmo();

        int wantedLevel = player->getWantedLevel();
        m_hud->update(playerData, wantedLevel, m_gameTime);
    }
    // Vehicle-to-Vehicle collision (Player-driven vs AI)Add commentMore actionsAdd commentMore actions
    if (player && player->isInVehicle() && carManager) {
        Vehicle* playerVehicle = player->getCurrentVehicle();
        if (playerVehicle) { // Should always be true if isInVehicle is true
            sf::FloatRect playerVehicleBounds = playerVehicle->getSprite().getGlobalBounds();

            for (auto& aiVehicle : carManager->getVehicles()) {
                if (&aiVehicle == playerVehicle || aiVehicle.hasDriver()) { // Don't check against self or other player-driven cars (if multiplayer later)
                    continue;
                }
                sf::FloatRect aiVehicleBounds = aiVehicle.getSprite().getGlobalBounds();
                if (playerVehicleBounds.intersects(aiVehicleBounds)) {
                    // Collision detected!
                    // Simple response: stop both vehicles for now.
                    playerVehicle->stop(); // stop() method sets speed to 0
                    aiVehicle.stop();
                    // Could add sound, visual effect, damage later
    // std::cout << "Player vehicle collided with AI vehicle!" << std::endl; // Commented out for less console spam
                }
            }
        }
    }

}

void GameManager::render() {
    window.clear(sf::Color::Black);
    window.setView(gameView);
    window.draw(mapSprite);
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

    for (const auto& bullet_ptr : bulletPool.getAllBullets()) { // Draw bullets from pool
        if (bullet_ptr->isActive()) {
            bullet_ptr->draw(window);
        }
    }

    window.setView(m_hudView);
    if (m_hud) m_hud->draw(window);

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

    //std::cout << "Loaded " << blockedPolygons.size() << " polygons\n";
  //  std::cout << "Loaded " << roads.size() << " road segments\n";
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
    pedestrianManager = GameFactory::createPedestrianManager(blockedPolygons);
    policeManager = GameFactory::createPoliceManager(*this, blockedPolygons);


    carManager = GameFactory::createCarManager(roads);

    //    for (int i = 0; i < 20; ++i) {
    carManager->spawnSingleVehicleOnRoad();
    //  }
    player = GameFactory::createPlayer(*this, { 100.f, 100.f }); // Pass *this (GameManager instance)
    //presents = GameFactory::createPresents(30, blockedPolygons);
    presents = GameFactory::createPresents(5, blockedPolygons);

    // Initialize HUD View
    m_hudView.setSize(static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y));
    m_hudView.setCenter(m_hudView.getSize().x / 2.f, m_hudView.getSize().y / 2.f);

    // Create and load HUD
    m_hud = std::make_unique<HUD>();
    std::string fontPath = "resources/Pricedown.otf"; // Make sure this path is correct
    std::string starTexturePath = "resources/star.png";   // Make sure this path is correct

    if (m_hud && !m_hud->loadResources(fontPath, starTexturePath)) {
        std::cerr << "CRITICAL: Failed to load HUD resources!" << std::endl;
        m_hud.reset(); // Disable HUD if loading failed
    }

    // Initial call to set HUD element positions based on the current view size
    if (m_hud) {
        m_hud->updateElementPositions(m_hudView.getSize().x, m_hudView.getSize().y);
    }

    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);
    gameView.setSize(winW, winH);
    gameView.zoom(0.25f);

    m_hud->updateElementPositions(window.getSize().x, window.getSize().y);

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
    gameView.setSize(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT));
    gameView.setCenter(gameView.getSize().x / 2.f, gameView.getSize().y / 2.f);
    window.setView(gameView);

    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);
    gameView.setSize(winW, winH);
    gameView.zoom(0.25f);



}


void GameManager::addBullet(const sf::Vector2f& startPos, const sf::Vector2f& direction) {
    Bullet* bullet = bulletPool.getBullet();
    if (bullet) {
        bullet->init(startPos, direction); // Call the new init method
    }
    else {
        // Optional: Log that the bullet pool is exhausted
        // std::cerr << "Bullet pool exhausted!" << std::endl;
    }
    // If bullet is nullptr, no bullet is fired. This is safer than crashing.
}