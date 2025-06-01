#include "GameManager.h"

#include <iostream>

GameManager::GameManager()
    : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Top-Down GTA Clone") {
    window.setFramerateLimit(60);


    // אתחול תפריט
    menu = std::make_unique<Menu>(window);
    currentState = GameState::Menu;
}

void GameManager::run() {
    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        processEvents();

        if (currentState == GameState::Menu) {
            menu->update();
            if (menu->isOptionChosen()) {
                std::string selected = menu->getSelectedOption();

                if (selected == "Start Game")
                    startGameFullscreen();
                    //currentState = GameState::Playing; (moved to startGameFullscreen)
                else if (selected == "Exit")
                    window.close();
                // Settings לא פעיל עדיין
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
    }
}

// GameManager.cpp

void GameManager::update(float deltaTime) {
    // 1️⃣ ראשית – עדכון השחקן (זז לפי קלט, ירי, כניסה/יציאה מהרכב וכן הלאה)
    player->update(deltaTime);

    // 2️⃣ עדכן צ'אנקים סביב מיקום השחקן
    chunkManager->updateChunks(player->getPosition());

    // 3️⃣ עדכון כל האובייקטים בתוך הצ'אנקים (למשל הולכי רגל)
    chunkManager->updateObjects(deltaTime);

    // 4️⃣ (אופציונלי) עדכון אובייקטים גלובליים נוספים כמו כדורים או פיצוצים
    // for (auto& obj : globalObjects)
    //     obj->update(deltaTime);

    // 5️⃣ עדכון מיקום המצלמה כך שתעקוב אחרי השחקן
    gameView.setCenter(player->getPosition());
}


// GameManager.cpp

void GameManager::render() {
    window.clear(sf::Color::Black);

    // 1️⃣ קבע View לעולם המשחק (Fullscreen, עוקב אחרי השחקן)
    window.setView(gameView);

    // 2️⃣ צייר את כל הצ'אנקים שנמצאים כרגע בזיכרון
    chunkManager->draw(window, gameView);

    // 3️⃣ (אופציונלי) צייר כאן אובייקטים גלובליים שלא בתוך צ'אנק מסוים
    // for (auto& obj : globalObjects)
    //     obj->draw(window);

    // 4️⃣ צייר את השחקן בסוף, כדי להבטיח שהוא מובלט מעל הצ'אנקים
    player->draw(window);

    // 5️⃣ (אופציונלי) לציור HUD
    // window.setView(window.getDefaultView());
    // uiManager->drawHUD(...);

    window.display();
}


void GameManager::startGameFullscreen() {
    // 1. נקבל את ה־Desktop Mode הנוכחי
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();

    // 2. “נשמיד” את החלון הקיים, וניצור אחד חדש במצב Fullscreen
    window.create(desktop, "Top-Down GTA Clone", sf::Style::Fullscreen);
    window.setFramerateLimit(60);

    // DEBUG: וידאו שהחלון באמת נפתח למסך מלא
    std::cout << "Switched to fullscreen, window size: "
        << window.getSize().x << "X" << window.getSize().y << "\n";

    // 3. כעת, אתחל את עולם המשחק – ChunkManager והשחקן
    chunkManager = std::make_unique<ChunkManager>();

    player = std::make_unique<Player>();
    // נניח שהיינו רוצים שמתחילים במרכז (0,0) – אפשר לשנות
    player->setPosition({ 0.f, 0.f });

    // 4. הגדר את ה־View כך שייקח את כל גודל המסך
    gameView.setSize(static_cast<float>(desktop.width), static_cast<float>(desktop.height));
    gameView.setCenter(player->getPosition());

    // 5. העבר את currentState למצב משחק
    currentState = GameState::Playing;
}
