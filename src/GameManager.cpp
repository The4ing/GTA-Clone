#include "GameManager.h"
#include <iostream> // For std::cerr
#include "ResourceManager.h"
#include "ResourceInitializer.h" // For loadGameResources
#include "Slideshow.h"
#include <nlohmann/json.hpp>
#include <SFML/System/Clock.hpp> // For sf::Clock
#include <SFML/System/Sleep.hpp> // For sf::sleep (optional, for testing loading screen)
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include "GameFactory.h"
#include "Vehicle.h"
#include <limits>
#include "player.h"
#include "InventoryUI.h"
#include "Constants.h"       // Required for MAP_BOUNDS
#include "PathfindingGrid.h"
#include "CollisionUtils.h"

using json = nlohmann::json;

// Helper function to calculate distance between two points (squared, to avoid sqrt initially)Add commentMore actions
float distanceSquared(const sf::Vector2f& p1, const sf::Vector2f& p2) {
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    return dx * dx + dy * dy;
}

GameManager::GameManager()
    : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Top-Down GTA Clone"), m_gameTime(sf::Time::Zero), m_isAwaitingFirstPlayerMove(false)
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
                if (selected == "Start Game") {
                    // --- Enhanced Loading Screen ---
                    window.clear();

                    // Background
                    sf::Sprite loadingBgSprite;
                    try {
                        loadingBgSprite.setTexture(ResourceManager::getInstance().getTexture("loading_background"));
                        // Scale background to fit window
                        sf::Vector2u texSize = loadingBgSprite.getTexture()->getSize();
                        sf::Vector2u winSize = window.getSize();
                        loadingBgSprite.setScale(
                            static_cast<float>(winSize.x) / texSize.x,
                            static_cast<float>(winSize.y) / texSize.y
                        );
                    }
                    catch (const std::runtime_error& e) {
                        std::cerr << "Error loading loading_background: " << e.what() << std::endl;
                        // Fallback: simple black background
                    }
                    window.draw(loadingBgSprite);

                    // Loading Text
                    sf::Text loadingText;
                    try {
                        loadingText.setFont(ResourceManager::getInstance().getFont("main"));
                        loadingText.setString("Loading Game...");
                        loadingText.setCharacterSize(50); // Increased size
                        loadingText.setFillColor(sf::Color::White);
                        // Center text
                        sf::FloatRect textRect = loadingText.getLocalBounds();
                        loadingText.setOrigin(textRect.left + textRect.width / 2.0f,
                            textRect.top + textRect.height / 2.0f);
                        loadingText.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f - 50.f); // Position above loading bar
                    }
                    catch (const std::runtime_error& e) {
                        std::cerr << "Error loading main font for loading text: " << e.what() << std::endl;
                    }
                    window.draw(loadingText);

                    // Loading Bar
                    float barWidth = 400.f;
                    float barHeight = 30.f;
                    sf::RectangleShape loadingBarBackground(sf::Vector2f(barWidth, barHeight));
                    loadingBarBackground.setFillColor(sf::Color(50, 50, 50)); // Dark gray
                    loadingBarBackground.setPosition(window.getSize().x / 2.0f - barWidth / 2.0f,
                        window.getSize().y / 2.0f + 20.f); // Position below text

                    sf::RectangleShape loadingBarFill(sf::Vector2f(0, barHeight)); // Start with 0 width
                    loadingBarFill.setFillColor(sf::Color(100, 200, 50)); // Light green
                    loadingBarFill.setPosition(loadingBarBackground.getPosition());

                    window.draw(loadingBarBackground);
                    window.draw(loadingBarFill);
                    window.display();

                    // Actually load resources
                    sf::Clock loadingClock; // Start timing for resource loading
                    ResourceInitializer::loadGameResources();
                    sf::Time resourceLoadTime = loadingClock.getElapsedTime();
                    std::cout << "Time to load game resources: " << resourceLoadTime.asSeconds() << "s\n";

                    // Simulate loading bar fill (for Option A)
                    // For a quick visual, just fill it after loading.
                    // An animation loop here would be more complex.
                    loadingBarFill.setSize(sf::Vector2f(barWidth, barHeight));
                    window.clear(); // Clear again before drawing the full bar
                    window.draw(loadingBgSprite);
                    window.draw(loadingText);
                    window.draw(loadingBarBackground);
                    window.draw(loadingBarFill);
                    window.display();
                    // sf::sleep(sf::seconds(0.5f)); // Optional: show full bar for a moment

                    loadingClock.restart(); // Restart clock for startGameFullscreen timing
                    startGameFullscreen();
                    sf::Time startGameSetupTime = loadingClock.getElapsedTime();
                    std::cout << "Time for startGameFullscreen setup: " << startGameSetupTime.asSeconds() << "s\n";

                }
                else if (selected == "Exit") {
                    window.close();
                }
            }

            if (currentState == GameState::Menu) { // Check if still in Menu state before drawing menu
                window.clear();
                menu->draw();
                window.display();
            }

        }
        else if (currentState == GameState::Playing) {
            update(deltaTime);
            render();
            if (m_playingFrameCount < 10) m_playingFrameCount++;
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
        else if (currentState == GameState::Store) {
            window.clear();
            // inventoryUI.draw(window, player->getInventory());
            window.setView(window.getDefaultView());  // הצגה לפי View רגיל של המסך
            for (auto& storePtr : store) {
                if (storePtr->getPlayerClose())
                    storePtr->open(*player);
                storePtr->handleInput(*player, window);
            }
            window.draw(frozenBackgroundSprite); // הרקע הקפואAdd commentMore actions


            // ואז ציור החנות
            for (auto& storePtr : store) {
                storePtr->drawUI(window);
            }

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

            window.setView(sf::View(sf::FloatRect(0.f, 0.f, w, h))); // חובה לעדכן גם את ה-View!

            if (m_hud) {
                m_hud->updateElementPositions(w, h);  // ← חשוב!
            }
        }


        if (event.type == sf::Event::KeyPressed) {
            if (m_isAwaitingFirstPlayerMove && currentState == GameState::Playing) {
                // Check for movement keys or any action key that should start the game
                if (event.key.code == sf::Keyboard::W ||
                    event.key.code == sf::Keyboard::A ||
                    event.key.code == sf::Keyboard::S ||
                    event.key.code == sf::Keyboard::D ||
                    event.key.code == sf::Keyboard::Up ||
                    event.key.code == sf::Keyboard::Down ||
                    event.key.code == sf::Keyboard::Left ||
                    event.key.code == sf::Keyboard::Right ||
                    event.key.code == sf::Keyboard::Space) { // Assuming space might be jump or action
                    m_isAwaitingFirstPlayerMove = false;
                    std::cout << "First player move detected, game is now active." << std::endl; // For debugging
                }
            }

            if (event.key.code == sf::Keyboard::F11)
                setFullscreen(!isFullscreen);
            if (currentState == GameState::Playing && (event.key.code == sf::Keyboard::I ||
                event.key.code == sf::Keyboard::E)) {
                // שמור את המצב הנוכחי של המשחק לתמונה
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
                m_playingFrameCount = 0; // Reset frame count when leaving playing state
                if (event.key.code == sf::Keyboard::I) {
                    currentState = GameState::Inventory;
                }
                else if (event.key.code == sf::Keyboard::E) {
                    for (auto& s : store) {
                        if (s->getPlayerClose()) {

                            currentState = GameState::Store;
                            break;
                        }
                    }
                }

            }
            else if ((currentState == GameState::Inventory || currentState == GameState::Store)
                && event.key.code == sf::Keyboard::Escape) {
                for (auto& s : store) {
                    s->setIsOpen(false);
                    s->setPlayerClose(false);
                }
                currentState = GameState::Playing; // m_playingFrameCount will naturally be 0 or start counting if it was reset
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

                        Vehicle* closestOverallVehicle = nullptr;
                        float minOverallDistanceSq = std::numeric_limits<float>::max();

                        // Check regular cars from CarManager
                        if (carManager) {
                            for (auto& car : carManager->getVehicles()) { // car is Vehicle&
                                if (car.getDriver() == nullptr) {
                                    float distSq = distanceSquared(playerPos, car.getPosition());
                                    if (distSq < enterRadiusSq && distSq < minOverallDistanceSq) {
                                        minOverallDistanceSq = distSq;
                                        closestOverallVehicle = &car;
                                    }
                                }
                            }
                        }

                        // Check police cars from PoliceManager
                        if (policeManager) {
                            for (const auto& policeCarUniquePtr : policeManager->getPoliceCars()) {
                                PoliceCar* policeCarCand = policeCarUniquePtr.get();
                                if (policeCarCand->getDriver() == nullptr) {
                                    float distSq = distanceSquared(playerPos, policeCarCand->getPosition());
                                    if (distSq < enterRadiusSq && distSq < minOverallDistanceSq) {
                                        if (player->getWantedLevel() >= 3) {
                                            // Player cannot enter a new police car if wanted level is 3+
                                            continue;
                                        }
                                        minOverallDistanceSq = distSq;
                                        closestOverallVehicle = policeCarCand;
                                    }
                                }
                            }
                        }

                        if (closestOverallVehicle) {
                            PoliceCar* policeCarScript = dynamic_cast<PoliceCar*>(closestOverallVehicle);

                            if (policeCarScript) {
                                // If it's a police car, and we've selected it,
                                // it implies player's wanted level was < 3 (due to the check in the loop).
                                if (player->getWantedLevel() < 3) {
                                    if (!policeCarScript->m_playerCausedWantedIncrease) {
                                        player->setWantedLevel(3);
                                        policeCarScript->m_playerCausedWantedIncrease = true;
                                    }
                                    policeCarScript->setIsAmbient(false); // Make it non-ambient
                                }
                                // If player's wanted level was already >= 3, this police car wouldn't have been chosen.
                            }

                            player->enterVehicle(closestOverallVehicle);
                            closestOverallVehicle->setDriver(player.get());
                        }
                    }
                }
            }
            if (currentState == GameState::Playing && event.key.code == sf::Keyboard::E) {

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
    for (auto& s : store)
        s->drawUI(target);
    for (const auto& bullet_ptr : bulletPool.getAllBullets()) { // Draw bullets from pool
        if (bullet_ptr->isActive()) {
            bullet_ptr->draw(target);
        }
    }
}

void GameManager::update(float dt) {
    sf::Clock frameTimer;
    bool timeThisFrame = (currentState == GameState::Playing && m_playingFrameCount < 10);

    // Player update always happens to catch the first input
    if (player) {
        player->update(dt, blockedPolygons);
    }
    else {
        return; // Early exit if player isn't initialized
    }

    if (!m_isAwaitingFirstPlayerMove) {
        // Update game time only if game has started
        m_gameTime += sf::seconds(dt * GAME_TIME_SCALE);

        if (carManager)
            carManager->update(dt, blockedPolygons);

        if (policeManager)
            policeManager->update(dt, *player, blockedPolygons);

        if (pedestrianManager)
            pedestrianManager->update(dt, blockedPolygons);

        for (auto& present : presents) // Assuming presents might have timed behavior or animations
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
            }
        }
        // Vehicle-to-Vehicle collision (Player-driven vs AI)
        if (player && player->isInVehicle() && carManager) {
            Vehicle* playerVehicle = player->getCurrentVehicle();
            if (playerVehicle) {
                sf::FloatRect playerVehicleBounds = playerVehicle->getSprite().getGlobalBounds();
                for (auto& aiVehicle : carManager->getVehicles()) {
                    if (&aiVehicle == playerVehicle || aiVehicle.hasDriver()) {
                        continue;
                    }
                    sf::FloatRect aiVehicleBounds = aiVehicle.getSprite().getGlobalBounds();
                    if (playerVehicleBounds.intersects(aiVehicleBounds)) {
                        playerVehicle->stop();
                        aiVehicle.stop();
                    }
                }
            }
        }
    }
    // This part handles view centering and should always run if player exists
    //  sf::Vector2f playerPos = player->getPosition();
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
        policeManager->update(dt, *player, blockedPolygons);


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
    for (auto& s : store) {
        float distSq = distanceSquared(player->getPosition(), s->getPosition());
        s->setPlayerClose(distSq < STORE_INTERACT_RADIUS * STORE_INTERACT_RADIUS);
    }

    if (timeThisFrame) {
        sf::Time updateTime = frameTimer.getElapsedTime();
        std::cout << "Frame " << m_playingFrameCount << " update time: " << updateTime.asSeconds() << "s\n";
    }
}

void GameManager::render() {
    sf::Clock frameTimer;
    bool timeThisFrame = (currentState == GameState::Playing && m_playingFrameCount < 10);

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

    for (auto& s : store) {
        s->drawUI(window);

    }




    for (auto& s : store) {
        if (s->getPlayerClose())
            window.draw(*s->getHintTextIfClose());
    }

    window.setView(window.getDefaultView());
    if (m_hud)
        m_hud->draw(window);





    window.display();

    if (timeThisFrame) {
        sf::Time renderTime = frameTimer.getElapsedTime();
        std::cout << "Frame " << m_playingFrameCount << " render time: " << renderTime.asSeconds() << "s\n";
    }


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

    std::cout << "Initializing PathfindingGrid..." << std::endl;
    pathfindingGrid = std::make_unique<PathfindingGrid>(MAP_BOUNDS, PATHFINDING_GRID_SIZE);
    pathfindingGrid->preprocess(blockedPolygons);
    std::cout << "PathfindingGrid preprocessing completed." << std::endl;

    mapTexture = &ResourceManager::getInstance().getTexture("map");
    mapSprite.setTexture(*mapTexture);
    pedestrianManager = GameFactory::createPedestrianManager(blockedPolygons);
    policeManager = GameFactory::createPoliceManager(*this, blockedPolygons);
    store = GameFactory::createStores(blockedPolygons);

    carManager = GameFactory::createCarManager(roads, *policeManager);

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
    m_isAwaitingFirstPlayerMove = true; // Start in the 'awaiting input' state
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
    // Update HUD view size and center
    m_hudView.setSize(winW, winH);
    m_hudView.setCenter(winW / 2.f, winH / 2.f);

    // Update HUD element positions based on new window size
    if (m_hud)
        m_hud->updateElementPositions(winW, winH);




}

void GameManager::addBullet(const sf::Vector2f& startPos, const sf::Vector2f& direction, BulletType type) {
    Bullet* bullet = bulletPool.getBullet();
    if (bullet) {
        bullet->init(startPos, direction, type);
    }
}


PathfindingGrid* GameManager::getPathfindingGrid() const {
    return pathfindingGrid.get();
}

bool GameManager::isPositionBlocked(const sf::Vector2f& pos) const {
    sf::FloatRect query(pos.x - 1.f, pos.y - 1.f, 2.f, 2.f);
    std::vector<const std::vector<sf::Vector2f>*> possible;
    blockedPolyTree.query(query, possible);
    for (const auto* poly : possible) {
        if (CollisionUtils::pointInPolygon(pos, *poly)) {
            return true;
        }
    }
    return false;
}