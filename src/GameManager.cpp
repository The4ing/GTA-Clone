#include "GameManager.h"
#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include "CollisionUtils.h"
#include "Constants.h"
#include "GameFactory.h"
#include "InventoryUI.h"
#include "PathfindingGrid.h"
#include "Player.h"
#include "ResourceInitializer.h"
#include "ResourceManager.h"
#include "Slideshow.h"
#include "Vehicle.h"

// Uncomment to time‑profile את 10 הפריימים הראשונים
// #define DEBUG_TIMING

using json = nlohmann::json;

/*‑‑‑‑‑  כלי עזר  ‑‑‑‑‑*/
static float distanceSquared(const sf::Vector2f& p1, const sf::Vector2f& p2)
{
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    return dx * dx + dy * dy;
}

/*‑‑‑‑‑  ctor  ‑‑‑‑‑*/
GameManager::GameManager()
    : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Top‑Down GTA Clone"),
    m_gameTime(sf::Time::Zero),
    m_isAwaitingFirstPlayerMove(false)
{
    frozenBackgroundTexture.create(window.getSize().x, window.getSize().y);
    window.setFramerateLimit(60);

    menu = std::make_unique<Menu>(window);
    currentState = GameState::Menu;

    /* Press‑any‑key overlay */
    try {
        m_pressStartText.setFont(ResourceManager::getInstance().getFont("main"));
        m_pressStartText.setString("Press any key to start");
        m_pressStartText.setCharacterSize(60);
        m_pressStartText.setFillColor(sf::Color::White);
    }
    catch (const std::exception& e) {
        std::cerr << "Error initializing start text: " << e.what() << '\n';
    }
    updatePressStartPosition();
}

/*‑‑‑‑‑  main loop  ‑‑‑‑‑*/
void GameManager::run()
{
    Slideshow slideshow(window, 2.5f);       // לוגו/קרדיטים בהפעלה – אם תרצה
    // slideshow.run();

    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        processEvents();

        /*‑‑‑ MENU ‑‑‑*/
        if (currentState == GameState::Menu) {
            if (menu->isOptionChosen()) {
                const std::string selected = menu->getSelectedOption();

                /*‑‑‑ Start Game ‑‑‑*/
                if (selected == "Start Game") {
                    /* שלב 1 – הצג בר טעינה ריק */
                    displayLoadingScreen("Loading Game...", 0.f);

                    sf::Clock loadingClock;

                    /* שלב 2 – טען משאבים כבדים */
                    ResourceInitializer::loadGameResources();

                    std::cout << "Time to load game resources: "
                        << loadingClock.getElapsedTime().asSeconds() << " s\n";

                    /* שלב 3 – הצג בר טעינה מלא */
                    displayLoadingScreen("Loading Game...", 1.f);

                    loadingClock.restart();
                    startGameFullscreen();

                    std::cout << "Time for startGameFullscreen setup: "
                        << loadingClock.getElapsedTime().asSeconds() << " s\n";
                }
                /*‑‑‑ Exit ‑‑‑*/
                else if (selected == "Exit") {
                    window.close();
                }
            }

            /* ציור התפריט */
            if (currentState == GameState::Menu) {
                window.clear();
                menu->draw();
                window.display();
            }
        }

        /*‑‑‑ PLAYING ‑‑‑*/
        else if (currentState == GameState::Playing) {
            update(deltaTime);
            render();
#ifdef DEBUG_TIMING
            if (m_playingFrameCount < 10) m_playingFrameCount++;
#endif
        }

        /*‑‑‑ INVENTORY ‑‑‑*/
        else if (currentState == GameState::Inventory) {
            window.clear();
            window.setView(window.getDefaultView());
            inventoryUI.handleInput(*player, player->getInventory(), window);
            window.draw(frozenBackgroundSprite);
            inventoryUI.draw(window, player->getInventory());
            window.display();
        }

        /*‑‑‑ STORE ‑‑‑*/
        else if (currentState == GameState::Store) {
            window.clear();
            window.setView(window.getDefaultView());

            for (auto& storePtr : store) {
                if (storePtr->getPlayerClose()) storePtr->open(*player);
                storePtr->handleInput(*player, window);
            }
            window.draw(frozenBackgroundSprite);

            for (auto& storePtr : store) storePtr->drawUI(window);
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
            updatePressStartPosition();
        }


        if (event.type == sf::Event::KeyPressed) {
            if (m_isAwaitingFirstPlayerMove && currentState == GameState::Playing) {
                // Check for movement keys or any action key that should start the game
                if (event.key.code != sf::Keyboard::F11) { // Ignore fullscreen toggle
                    std::cout << "First player move detected, game is now active." << std::endl; // For debugging
                    m_isAwaitingFirstPlayerMove = false;
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
#ifdef DEBUG_TIMING
    sf::Clock frameTimer;
    bool timeThisFrame = (currentState == GameState::Playing && m_playingFrameCount < 10);
#endif

    // --- 1. Update player (always runs to catch first input) ---
    if (!player) return;
    player->update(dt, blockedPolygons);

    // --- 2. If game started, update systems ---
    if (!m_isAwaitingFirstPlayerMove) {
        m_gameTime += sf::seconds(dt * GAME_TIME_SCALE);

        if (carManager)
            carManager->update(dt, blockedPolygons);

        if (policeManager)
            policeManager->update(dt, *player, blockedPolygons);

        if (pedestrianManager)
            pedestrianManager->update(dt, blockedPolygons);

        for (auto& present : presents)
            present->update(dt, blockedPolygons);

        std::vector<Pedestrian*> npcPtrs;
        if (pedestrianManager) {
            for (const auto& up : pedestrianManager->getPedestrians())
                npcPtrs.push_back(up.get());
        }
        std::vector<Police*> policePtrs;
        if (policeManager) {
            for (const auto& p : policeManager->getPoliceOfficers())
                policePtrs.push_back(p.get());
        }
        std::vector<Vehicle*> carPtrs;
        if (carManager) {
            for (auto& v : carManager->getVehicles())
                carPtrs.push_back(&v);
        }

        player->getShooter().update(dt, blockedPolygons, npcPtrs, policePtrs, carPtrs);

        // Vehicle-to-Vehicle collision (Player-driven vs AI)
        if (player->isInVehicle() && carManager) {
            Vehicle* playerVehicle = player->getCurrentVehicle();
            if (playerVehicle) {
                sf::FloatRect playerBounds = playerVehicle->getSprite().getGlobalBounds();
                for (auto& aiVehicle : carManager->getVehicles()) {
                    if (&aiVehicle == playerVehicle || aiVehicle.hasDriver())
                        continue;
                    if (playerBounds.intersects(aiVehicle.getSprite().getGlobalBounds())) {
                        playerVehicle->stop();
                        aiVehicle.stop();
                    }
                }
            }
        }
    }

    // --- 3. Camera follow player or vehicle ---
    sf::Vector2f focusPosition = player->isInVehicle() && player->getCurrentVehicle()
        ? player->getCurrentVehicle()->getPosition()
        : player->getPosition();

    sf::Vector2f viewSize = gameView.getSize();
    float halfW = std::min(viewSize.x * 0.5f, MAP_WIDTH * 0.5f);
    float halfH = std::min(viewSize.y * 0.5f, MAP_HEIGHT * 0.5f);

    sf::Vector2f newCenter = focusPosition;
    newCenter.x = std::clamp(newCenter.x, halfW, MAP_WIDTH - halfW);
    newCenter.y = std::clamp(newCenter.y, halfH, MAP_HEIGHT - halfH);
    gameView.setCenter(newCenter);

    // --- 4. Collision with presents ---
    for (auto& present : presents) {
        if (!present->isCollected() &&
            player->getCollisionBounds().intersects(present->getSprite().getGlobalBounds())) {
            player->onCollision(*present);  // Double Dispatch
        }
    }

    // --- 5. HUD Update ---
    if (m_hud && currentState == GameState::Playing) {
        PlayerData data;
        data.money = player->getMoney();
        data.health = player->getHealth();
        data.armor = player->getArmor();
        data.weaponName = player->getCurrentWeaponName();
        data.currentAmmo = player->getCurrentAmmo(data.weaponName);
        data.maxAmmo = player->getMaxAmmo();

        m_hud->update(data, player->getWantedLevel(), m_gameTime);
    }

    // --- 6. Store Proximity Check ---
    for (auto& s : store) {
        float distSq = distanceSquared(player->getPosition(), s->getPosition());
        s->setPlayerClose(distSq < STORE_INTERACT_RADIUS * STORE_INTERACT_RADIUS);
    }

    // --- 7. Frame Timing Debug ---
#ifdef DEBUG_TIMING
    if (timeThisFrame) {
        sf::Time updateTime = frameTimer.getElapsedTime();
        std::cout << "Frame " << m_playingFrameCount << " update time: " << updateTime.asSeconds() << "s\n";
    }
#endif
}


void GameManager::render() {
#ifdef DEBUG_TIMING
    sf::Clock frameTimer;
    bool timeThisFrame = (currentState == GameState::Playing && m_playingFrameCount < 10);
#endif

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
    if (m_isAwaitingFirstPlayerMove) {
        sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
        overlay.setFillColor(sf::Color(0, 0, 0, 150));
        window.draw(overlay);
        window.draw(m_pressStartText);
    }




    window.display();

#ifdef DEBUG_TIMING
    if (timeThisFrame) {
        sf::Time renderTime = frameTimer.getElapsedTime();
        std::cout << "Frame " << m_playingFrameCount << " render time: " << renderTime.asSeconds() << "s\n";
    }
#endif

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
    CollisionUtils::setBlockedPolyTree(&blockedPolyTree);
}




void GameManager::startGameFullscreen() {
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    window.create(desktop, "Top-Down GTA Clone", sf::Style::Fullscreen);
    window.setFramerateLimit(60);
    isFullscreen = true;

    updatePressStartPosition();

    // Display a simple loading screen while heavy resources are initialized
    try {
        // Call the new helper function. Progress is 0 initially for this stage.
        displayLoadingScreen("Loading...", 0.0f);
    }
    catch (const std::exception& e) { // Catch specific sf::RuntimeError or std::exception
        std::cerr << "Error displaying initial loading screen in startGameFullscreen: " << e.what() << std::endl;
        // If resources for loading screen itself are missing, just clear screen
        window.clear(sf::Color::Black); // Clear with a fallback color
        window.display();
    }
    // Note: The actual resource loading for the game happens after this.
    // The old code here was only for the visual display of "Loading..."

    gameView.setSize(static_cast<float>(desktop.width), static_cast<float>(desktop.height));
    gameView.setCenter(gameView.getSize().x / 2.f, gameView.getSize().y / 2.f);
    window.setView(gameView);
    updatePressStartPosition();


    loadCollisionRectsFromJSON("resources/map.tmj");
    buildBlockedPolyTree();

    std::cout << "Initializing PathfindingGrid..." << std::endl;
    pathfindingGrid = std::make_unique<PathfindingGrid>(MAP_BOUNDS, PATHFINDING_GRID_SIZE);
    pathfindingGrid->preprocess(blockedPolygons);
    std::cout << "PathfindingGrid preprocessing completed." << std::endl;

    mapTexture = &ResourceManager::getInstance().getTexture("map");
    mapSprite.setTexture(*mapTexture);
    setupPatrolZones(); // Call to define patrol zones
    pedestrianManager = GameFactory::createPedestrianManager(blockedPolygons);
    policeManager = GameFactory::createPoliceManager(*this, blockedPolygons);
    if (policeManager)
        //    policeManager->spawnStaticPoliceUnits(MAP_BOUNDS, STATIC_POLICE_GRID_SIZE, blockedPolygons);
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

const std::vector<PatrolZone>& GameManager::getPatrolZones() const {
    return m_patrolZones;
}

void GameManager::setupPatrolZones() {
    // Placeholder: Define a few patrol zones
    // These coordinates and radii are arbitrary and need to be adjusted for your map.
    // Zone IDs should be unique.
    m_patrolZones.emplace_back(0, sf::Vector2f(500.f, 500.f), 300.f);  // Zone 0
    m_patrolZones.emplace_back(1, sf::Vector2f(1500.f, 800.f), 250.f); // Zone 1
    m_patrolZones.emplace_back(2, sf::Vector2f(800.f, 1500.f), 400.f); // Zone 2
    // ... add more zones to cover the map

    // TODO:
    // For each zone, spawn initial foot police and optionally a patrol car using PoliceManager.
    // This requires PoliceManager to have methods to spawn units and assign them to a zone.
    // Example (conceptual, actual methods might differ):
    // if (policeManager) {
    //     for (PatrolZone& zone : m_patrolZones) {
    //         // Spawn 2 foot officers for this zone
    //         for (int i = 0; i < 2; ++i) {
    //             // Find a valid spawn point within the zone
    //             sf::Vector2f spawnPos = zone.center + sf::Vector2f(rand() % 100 - 50, rand() % 100 - 50);
    //             Police* officer = policeManager->spawnPatrolOfficer(spawnPos, &zone); // spawnPatrolOfficer needs to exist
    //             if (officer) {
    //                 zone.assignedFootPatrols.push_back(officer);
    //             }
    //         }
    //         // Spawn 1 patrol car for this zone (50% chance)
    //         if (rand() % 2 == 0) {
    //             sf::Vector2f carSpawnPos = zone.center + sf::Vector2f(rand() % 50 - 25, rand() % 50 - 25); // Near center
    //             // Find nearest road for carSpawnPos
    //             // PoliceCar* car = policeManager->spawnPatrolCar(carSpawnPos, &zone); // spawnPatrolCar needs to exist
    //             // if (car) {
    //             //     zone.assignedPatrolCar = car;
    //             // }
    //         }
    //     }
    // }
    std::cout << "Patrol zones set up. Count: " << m_patrolZones.size() << std::endl;
}

void GameManager::updatePressStartPosition() {
    sf::FloatRect textRect = m_pressStartText.getLocalBounds();
    m_pressStartText.setOrigin(textRect.left + textRect.width / 2.f,
        textRect.top + textRect.height / 2.f);
    m_pressStartText.setPosition(window.getSize().x / 2.f,
        window.getSize().y / 2.f);
}

void GameManager::displayLoadingScreen(const std::string& message, float initialProgress) {
    window.clear();

    // Background
    sf::Sprite loadingBgSprite;
    try {
        loadingBgSprite.setTexture(ResourceManager::getInstance().getTexture("loading_background"));
        sf::Vector2u texSize = loadingBgSprite.getTexture()->getSize();
        sf::Vector2u winSize = window.getSize();
        loadingBgSprite.setScale(
            static_cast<float>(winSize.x) / texSize.x,
            static_cast<float>(winSize.y) / texSize.y
        );
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Error loading loading_background: " << e.what() << std::endl;
        // Fallback: simple black background if texture fails
    }
    window.draw(loadingBgSprite);

    // Loading Text
    sf::Text loadingText;
    try {
        loadingText.setFont(ResourceManager::getInstance().getFont("main"));
        loadingText.setString(message);
        loadingText.setCharacterSize(50);
        loadingText.setFillColor(sf::Color::White);
        sf::FloatRect textRect = loadingText.getLocalBounds();
        loadingText.setOrigin(textRect.left + textRect.width / 2.0f,
            textRect.top + textRect.height / 2.0f);
        loadingText.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f - 50.f);
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
        window.getSize().y / 2.0f + 20.f);

    sf::RectangleShape loadingBarFill(sf::Vector2f(barWidth * initialProgress, barHeight));
    loadingBarFill.setFillColor(sf::Color(100, 200, 50)); // Light green
    loadingBarFill.setPosition(loadingBarBackground.getPosition());

    window.draw(loadingBarBackground);
    window.draw(loadingBarFill);
    window.display();
}