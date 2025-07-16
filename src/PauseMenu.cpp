#include "PauseMenu.h"
#include "Constants.h" 
#include "ResourceManager.h" 
#include "SoundManager.h" 
#include <iostream> 
#include <cmath>

// It's good practice to put Constants.h inclusion within an ifdef
// or ensure it's included correctly by the build system.
// For now, we'll assume it defines SCREEN_WIDTH and SCREEN_HEIGHT if used.

PauseMenu::PauseMenu() : m_isOpen(false), selectedIndex(0), selectedOption(MenuOption::Resume), showingMap(false), showingStats(false),
   destinationBlinkInterval(0.5f), newGameConfirmIndex(0), statsDataLoaded(false), showingNewGameConfirm(false),mapResourcesInitialized(false),
  wasEscapePressed(false) {
   
    font = ResourceManager::getInstance().getFont("main");

    menuItems = { "Resume Game", "Start New Game", "Map", "Stats", "Volume Up", "Volume Down", "Mute", "Exit" };

    playerMarker.setRadius(8.f);
    playerMarker.setFillColor(sf::Color::Blue);
    playerMarker.setOutlineColor(sf::Color::White);
    playerMarker.setOutlineThickness(2.f);
    playerMarker.setOrigin(playerMarker.getRadius(), playerMarker.getRadius());
    destinationMarker.setRadius(20.f);
    destinationMarker.setFillColor(sf::Color::Red);
    destinationMarker.setOrigin(destinationMarker.getRadius(), destinationMarker.getRadius());

    titleText.setFont(font);
    titleText.setString("Paused");
    titleText.setCharacterSize(50); // Adjust size as needed
    titleText.setFillColor(sf::Color::White);
    // Centering will be done in draw() to adapt to window size

    menuText.setFont(font);
    menuText.setCharacterSize(50); // Adjust size as needed
    menuText.setFillColor(sf::Color::White);

    background.setFillColor(sf::Color(0, 0, 0, 170));

    // Initialize New Game Confirmation Dialog elements
    confirmDialogBackground.setFillColor(sf::Color(50, 50, 50, 220)); // Darker, more opaque
    confirmDialogBackground.setOutlineColor(sf::Color::White);
    confirmDialogBackground.setOutlineThickness(2.f);

    confirmDialogText.setFont(font);
    confirmDialogText.setCharacterSize(50);
    confirmDialogText.setFillColor(sf::Color::White);

    confirmOptionText.setFont(font);
    confirmOptionText.setCharacterSize(50);

    volumeLevelText.setFont(font);
    volumeLevelText.setCharacterSize(35); // Smaller text for status
    volumeLevelText.setFillColor(sf::Color::White);
}

void PauseMenu::open() {
    m_isOpen = true;
    selectedIndex = 0;
    escCooldownClock.restart();
    selectedOption = MenuOption::Resume;
    showingMap = false;
    showingStats = false;
    showingNewGameConfirm = false; // Ensure confirm dialog is not shown on open
    updateVolumeDisplayText(); // Set initial volume display
    SoundManager::getInstance().pauseAll();
    std::cout << "PauseMenu opened." << std::endl;
}

void PauseMenu::close() {
    m_isOpen = false;
    showingMap = false;
    showingStats = false;
    SoundManager::getInstance().resumeAll();
    std::cout << "PauseMenu closed." << std::endl; // For debugging
    // Consider resuming game-specific sounds/music here
}

bool PauseMenu::isOpen() const {
    return m_isOpen;
}

void PauseMenu::update(float dt) {
    if (!m_isOpen) return;

    // Currently, no dynamic updates needed for the menu itself when it's just static items.
    // This could be used for animations or timed events within the pause menu later.
}

void PauseMenu::draw(sf::RenderTarget& target) {
    if (!m_isOpen) return;

    // Set background size to cover the entire render target
    background.setSize(sf::Vector2f(static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y)));
    target.draw(background);

    // Center title text
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(titleBounds.left + titleBounds.width / 2.0f, titleBounds.top + titleBounds.height / 2.0f);
    titleText.setPosition(static_cast<float>(target.getSize().x) / 2.0f, static_cast<float>(target.getSize().y) / 4.0f);
    target.draw(titleText);

    float currentY = static_cast<float>(target.getSize().y) / 2.5f;
    float itemSpacing = 45.f;
    if (!showingMap && !showingStats) {
        for (size_t i = 0; i < menuItems.size(); ++i) {
            menuText.setString(menuItems[i]);
            sf::FloatRect itemBounds = menuText.getLocalBounds();
            menuText.setOrigin(itemBounds.left + itemBounds.width / 2.0f, itemBounds.top + itemBounds.height / 2.0f);
            menuText.setPosition(static_cast<float>(target.getSize().x) / 2.0f, currentY);

            if (static_cast<int>(i) == selectedIndex) {
                menuText.setFillColor(sf::Color::Yellow);
                menuText.setStyle(sf::Text::Bold);
            }
            else {
                menuText.setFillColor(sf::Color::White);
                menuText.setStyle(sf::Text::Regular);
            }
            target.draw(menuText);
            currentY += itemSpacing;
        }
    }

    // Draw volume level text if no sub-screen or dialog is active
    if (!showingMap && !showingStats && !showingNewGameConfirm) {
        sf::FloatRect volumeTextBounds = volumeLevelText.getLocalBounds();
        // Position it below the last menu item, centered.
        // currentY here is already past the last item due to loop increment.
        volumeLevelText.setOrigin(volumeTextBounds.left + volumeTextBounds.width / 2.f, 0.f);
        volumeLevelText.setPosition(target.getSize().x / 2.f, currentY + itemSpacing * 0.5f);
        target.draw(volumeLevelText);
    }

    // Later, if showingMap or showingStats is true, this is where you'd draw those screens.
    // The NewGameConfirm dialog is drawn after these, at the very end of this function.
    if (showingMap) {
        if (mapResourcesInitialized) {
            sf::View previousView = target.getView();
            target.setView(mapDisplayView);

            target.draw(mapDisplaySprite);

            // Player marker position should be updated if player can move while map is open (not current case)
            // For now, its position is set in prepareMapScreen and on pan/zoom (if view center changes)
            playerMarker.setPosition(playerMapPosition);
            target.draw(playerMarker);
            bool blinkVisible = fmod(destinationBlinkClock.getElapsedTime().asSeconds(),
                destinationBlinkInterval * 2.f) < destinationBlinkInterval;
            for (const auto& p : missionPoints) {
                if (blinkVisible) {
                    destinationMarker.setPosition(p.second);
                    target.draw(destinationMarker);
                }
            }
            target.setView(previousView);

            // Draw instructions in the default view (or a HUD view)
            sf::Text mapInstructions("Map - Arrows/WASD: Pan | +/- or Scroll: Zoom | ESC: Close", font, 18);
            mapInstructions.setCharacterSize(50);
            mapInstructions.setFillColor(sf::Color::White);
            // Position at bottom-center or bottom-left
            sf::FloatRect instrBounds = mapInstructions.getLocalBounds();
            mapInstructions.setOrigin(instrBounds.left + instrBounds.width / 2.0f, 0); // Center horizontally
            mapInstructions.setPosition(target.getSize().x / 2.0f, target.getSize().y - mapInstructions.getCharacterSize() - 10.f);
            target.draw(mapInstructions);

        }
        else {
            sf::Text errorText("Map resources not loaded!", font, 30);
            errorText.setFillColor(sf::Color::Red);
            sf::FloatRect bounds = errorText.getLocalBounds();
            errorText.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
            errorText.setPosition(target.getSize().x / 2.f, target.getSize().y / 2.f);
            target.draw(errorText);
        }
    }
    else if (showingStats) {
        if (statsDataLoaded) {
            // Assuming default view is active from main pause menu drawing.
            // If not, set target.setView(target.getDefaultView());

            float currentY = target.getSize().y / 4.8f; // Start Y position for stats
            float lineSpacing = 38.f;
            float leftMargin = target.getSize().x / 3.8f; // Left margin for labels
            float valueOffsetX = 300.f; // X offset for values from labels

            sf::Text statsTitleText("Player Statistics", font, 40);
            statsTitleText.setCharacterSize(45);
            statsTitleText.setFillColor(sf::Color::White);
            statsTitleText.setStyle(sf::Text::Bold);
            sf::FloatRect titleBounds = statsTitleText.getLocalBounds();
            statsTitleText.setOrigin(titleBounds.left + titleBounds.width / 2.0f, titleBounds.top + titleBounds.height / 2.0f);
            statsTitleText.setPosition(target.getSize().x / 2.0f, currentY);
            target.draw(statsTitleText);
            currentY += lineSpacing * 1.8f;

            auto drawStatLine = [&](const std::string& label, const std::string& valueStr) {
                sf::Text statLabel(label, font, 26);
                statLabel.setCharacterSize(45);
                statLabel.setFillColor(sf::Color(210, 210, 210)); // Light gray for labels
                statLabel.setPosition(leftMargin, currentY);
                target.draw(statLabel);

                sf::Text statValue(valueStr, font, 26);
                statValue.setCharacterSize(45);
                statValue.setFillColor(sf::Color::White);
                statValue.setPosition(leftMargin + valueOffsetX, currentY);
                target.draw(statValue);
                currentY += lineSpacing;
                };

            int totalSeconds = static_cast<int>(displayedStats.gameTime.asSeconds());
            int minutes = totalSeconds / 60;
            int seconds = totalSeconds % 60;
            char timeStrBuffer[16]; // Buffer for time string
            snprintf(timeStrBuffer, sizeof(timeStrBuffer), "%02d:%02d", minutes, seconds);
            drawStatLine("Game Time:", timeStrBuffer);

            drawStatLine("Kills:", std::to_string(displayedStats.kills));
            drawStatLine("Money:", "$" + std::to_string(displayedStats.money));
            drawStatLine("Wanted Level:", std::to_string(displayedStats.wantedLevel) + (displayedStats.wantedLevel == 1 ? " Star" : " Stars"));

            currentY += lineSpacing * 0.2f; // Small gap
            sf::Text weaponsTitle("Collected Weapons:", font, 26);
            weaponsTitle.setCharacterSize(45);
            weaponsTitle.setFillColor(sf::Color(210, 210, 210));
            weaponsTitle.setPosition(leftMargin, currentY);
            target.draw(weaponsTitle);
            currentY += lineSpacing;

            if (displayedStats.collectedWeapons.empty()) {
                sf::Text noWeaponsText("None", font, 24);
                noWeaponsText.setCharacterSize(45);
                noWeaponsText.setFillColor(sf::Color::White);
                noWeaponsText.setPosition(leftMargin + 30.f, currentY); // Indent a bit
                target.draw(noWeaponsText);
            }
            else {
                float initialWeaponY = currentY;
                for (size_t i = 0; i < displayedStats.collectedWeapons.size(); ++i) {
                    // Basic pagination for weapons list if too long
                    if (i > 0 && i % 4 == 0) { // Show 4 weapons then "more" or new column
                        if (leftMargin + 30.f + 200.f < target.getSize().x * 0.8f) { // Check if space for another column
                            leftMargin += 200.f; // Start new column
                            currentY = initialWeaponY;
                        }
                        else if (currentY > target.getSize().y - lineSpacing * 2.f) { // Check if too low on screen
                            sf::Text moreWeaponsText("...and more", font, 20);
                            moreWeaponsText.setCharacterSize(45);
                            moreWeaponsText.setFillColor(sf::Color(128, 128, 128));
                            moreWeaponsText.setPosition(leftMargin + 30.f, currentY);
                            target.draw(moreWeaponsText);
                            break;
                        }
                    }
                    if (currentY > target.getSize().y - lineSpacing * 2.f && i < displayedStats.collectedWeapons.size() - 1) {
                        sf::Text moreWeaponsText("...and more", font, 20);
                        moreWeaponsText.setFillColor(sf::Color(128, 128, 128));
                        moreWeaponsText.setPosition(leftMargin + 30.f, currentY);
                        target.draw(moreWeaponsText);
                        break;
                    }

                    sf::Text weaponText("- " + displayedStats.collectedWeapons[i], font, 22);
                    weaponText.setCharacterSize(45);
                    weaponText.setFillColor(sf::Color::White);
                    weaponText.setPosition(leftMargin + 30.f, currentY);
                    target.draw(weaponText);
                    currentY += (lineSpacing * 0.75f);
                }
            }
            // Reset Y for instruction text
            currentY = target.getSize().y - 50.f;
            sf::Text statsInstructions("ESC to Close", font, 18);
            statsInstructions.setCharacterSize(45);
            statsInstructions.setFillColor(sf::Color::White);
            sf::FloatRect instrBoundsStats = statsInstructions.getLocalBounds();
            statsInstructions.setOrigin(instrBoundsStats.left + instrBoundsStats.width / 2.0f, 0.f);
            statsInstructions.setPosition(target.getSize().x / 2.0f, currentY); // Position at bottom
            target.draw(statsInstructions);

        }
        else {
            // Error text if data not loaded
            sf::Text errorText("Stats data not available!", font, 30);
            errorText.setFillColor(sf::Color::Red);
            sf::FloatRect bounds = errorText.getLocalBounds();
            errorText.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
            errorText.setPosition(target.getSize().x / 2.f, target.getSize().y / 2.f);
            target.draw(errorText);
        }
    }

    // Draw New Game Confirmation Dialog if active (on top of everything else in pause menu)
    if (showingNewGameConfirm) {
        sf::Vector2u windowSize = target.getSize();
        float dialogWidth = windowSize.x * 0.5f;
        float dialogHeight = windowSize.y * 0.3f;
        confirmDialogBackground.setSize(sf::Vector2f(dialogWidth, dialogHeight));
        confirmDialogBackground.setOrigin(dialogWidth / 2.f, dialogHeight / 2.f);
        confirmDialogBackground.setPosition(windowSize.x / 2.f, windowSize.y / 2.f);
        target.draw(confirmDialogBackground);

        confirmDialogText.setString("Start a new game?\nUnsaved progress will be lost.");
        sf::FloatRect textBounds = confirmDialogText.getLocalBounds();
        confirmDialogText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
        confirmDialogText.setPosition(windowSize.x / 2.f, windowSize.y / 2.f - dialogHeight * 0.2f);
        target.draw(confirmDialogText);

        float optionY = windowSize.y / 2.f + dialogHeight * 0.25f;
        float optionSpacing = 150.f;

        for (size_t i = 0; i < newGameConfirmOptions.size(); ++i) {
            confirmOptionText.setString(newGameConfirmOptions[i]);
            textBounds = confirmOptionText.getLocalBounds();
            confirmOptionText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);

            // Position Yes/No side-by-side
            float optionX = windowSize.x / 2.f + (static_cast<float>(i) - 0.5f) * optionSpacing;
            confirmOptionText.setPosition(optionX, optionY);

            if (static_cast<int>(i) == newGameConfirmIndex) {
                confirmOptionText.setFillColor(sf::Color::Yellow);
                confirmOptionText.setStyle(sf::Text::Bold);
            }
            else {
                confirmOptionText.setFillColor(sf::Color::White);
                confirmOptionText.setStyle(sf::Text::Regular);
            }
            target.draw(confirmOptionText);
        }
    }
}

void PauseMenu::handleEvent(const sf::Event& event) {
    if (!m_isOpen) return;

    // Handle New Game Confirmation Dialog first if it's active
    if (showingNewGameConfirm) {
        if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
            case sf::Keyboard::Left:
            case sf::Keyboard::A:
                newGameConfirmIndex = 0; // "Yes"
                break;
            case sf::Keyboard::Right:
            case sf::Keyboard::D:
                newGameConfirmIndex = 1; // "No"
                break;
            case sf::Keyboard::Enter:
            case sf::Keyboard::Space:
                if (newGameConfirmIndex == 0) {
                    m_currentAction = MenuAction::RequestNewGame;
                    showingNewGameConfirm = false;
                    close();
                    std::cout << "New Game confirmed." << std::endl;
                }
                else {
                    showingNewGameConfirm = false;
                    newGameConfirmIndex = 1;
                    std::cout << "New Game cancelled." << std::endl;
                }
                break;
            case sf::Keyboard::Escape:
                showingNewGameConfirm = false;
                newGameConfirmIndex = 1;
                std::cout << "New Game cancelled by ESC." << std::endl;
                break;
            default:
                break;
            }
        }
        return;
    }

    // Handle map/stats views
    if (showingMap || showingStats) {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            if (showingMap) {
                showingMap = false;
                mapResourcesInitialized = false;
                std::cout << "Map view closed by ESC." << std::endl;
                return;
            }
            if (showingStats) {
                showingStats = false;
                statsDataLoaded = false;
                std::cout << "Stats view closed by ESC." << std::endl;
                return;
            }
        }

        if (showingMap && mapResourcesInitialized) {
            float moveSpeed = 30.f;
            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                case sf::Keyboard::Left:
                case sf::Keyboard::A:
                    mapDisplayView.move(-moveSpeed, 0);
                    break;
                case sf::Keyboard::Right:
                case sf::Keyboard::D:
                    mapDisplayView.move(moveSpeed, 0);
                    break;
                case sf::Keyboard::Up:
                case sf::Keyboard::W:
                    mapDisplayView.move(0, -moveSpeed);
                    break;
                case sf::Keyboard::Down:
                case sf::Keyboard::S:
                    mapDisplayView.move(0, moveSpeed);
                    break;
                case sf::Keyboard::Add:
                case sf::Keyboard::Equal:
                    mapDisplayView.zoom(0.85f);
                    break;
                case sf::Keyboard::Subtract:
                case sf::Keyboard::Dash:
                    mapDisplayView.zoom(1.15f);
                    break;
                default:
                    break;
                }
            }
            else if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                    if (event.mouseWheelScroll.delta > 0)
                        mapDisplayView.zoom(0.9f);
                    else if (event.mouseWheelScroll.delta < 0)
                        mapDisplayView.zoom(1.1f);
                }
            }
        }
        return;
    }

    else if (showingStats) {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            showingStats = false;
            statsDataLoaded = false;
            std::cout << "Stats view closed by ESC." << std::endl;
            return;
        }
        if (event.type == sf::Event::KeyPressed ||
            event.type == sf::Event::MouseButtonPressed ||
            event.type == sf::Event::MouseWheelScrolled) {
            return;
        }
    }

    // Main pause menu input
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
        case sf::Keyboard::Up:
        case sf::Keyboard::W:
            navigateUp();
            break;
        case sf::Keyboard::Down:
        case sf::Keyboard::S:
            navigateDown();
            break;
        case sf::Keyboard::Enter:
        case sf::Keyboard::Space:
            selectCurrent();
            break;
        case sf::Keyboard::Escape:
          
            if (escCooldownClock.getElapsedTime().asMilliseconds() < 150) {
                
                return;
            }
            if (!wasEscapePressed) {
                std::cout << "PauseMenu: ESC pressed, calling close()." << std::endl;
                close();
                wasEscapePressed = true;
            }
            break;
        default:
            break;
        }
    }
    else if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Escape) {
        wasEscapePressed = false;
    }
}


void PauseMenu::navigateUp() {
    selectedIndex--;
    if (selectedIndex < 0) {
        selectedIndex = static_cast<int>(menuItems.size()) - 1;
    }
    selectedOption = static_cast<MenuOption>(selectedIndex);
    std::cout << "Navigated Up to: " << menuItems[selectedIndex] << std::endl; // Debug
}

void PauseMenu::navigateDown() {
    selectedIndex++;
    if (selectedIndex >= static_cast<int>(menuItems.size())) {
        selectedIndex = 0;
    }
    selectedOption = static_cast<MenuOption>(selectedIndex);
    std::cout << "Navigated Down to: " << menuItems[selectedIndex] << std::endl; // Debug
}

void PauseMenu::selectCurrent() {
    selectedOption = static_cast<MenuOption>(selectedIndex);
    std::cout << "Selected option: " << menuItems[selectedIndex] << std::endl;
    // m_currentAction = MenuAction::None; // Reset action by default - getAndClearAction handles reset

    switch (selectedOption) {
    case MenuOption::Resume:
        // m_currentAction = MenuAction::Resume; // GameManager primarily uses isOpen() for resume
        close();
        SoundManager::getInstance().resumeAll();
        break;
    case MenuOption::NewGame:
        // Instead of immediate action, show confirmation
        showingNewGameConfirm = true;
        newGameConfirmIndex = 1; // Default to "No" (index 1 for {"Yes", "No"})
        std::cout << "Showing New Game confirmation." << std::endl;
        break;
    case MenuOption::Map:
        m_currentAction = MenuAction::RequestOpenMap;
        std::cout << "Action: RequestOpenMap" << std::endl;
        // showingMap will be set by prepareMapScreen
        break;
    case MenuOption::Stats:
        m_currentAction = MenuAction::RequestOpenStats;
        // showingStats will be set by prepareStatsScreen
        std::cout << "Action: RequestOpenStats" << std::endl;
        break;
    case MenuOption::VolumeUp:
        std::cout << "Action: Volume Up" << std::endl;
        SoundManager::getInstance().increaseVolume(5.f);
        updateVolumeDisplayText();
        break;
    case MenuOption::VolumeDown:
        std::cout << "Action: Volume Down" << std::endl;
        SoundManager::getInstance().decreaseVolume(5.f);
        updateVolumeDisplayText();
        break;
    case MenuOption::Mute:
        std::cout << "Action: Toggle Mute" << std::endl;
        SoundManager::getInstance().toggleMute();
        updateVolumeDisplayText(); // Update general volume text
        // Update the Mute/Unmute text on the menu item itself
        for (size_t i = 0; i < menuItems.size(); ++i) {
            if (static_cast<MenuOption>(i) == MenuOption::Mute) {
                menuItems[i] = SoundManager::getInstance().isMuted() ? "Unmute" : "Mute";
                break;
            }
        }
        break;
    case MenuOption::Exit:
        m_currentAction = MenuAction::Exit;
        std::cout << "Action: Exit" << std::endl;
        break;
    case MenuOption::Count: // Should not be selectable
        break;
    }
}

PauseMenu::MenuAction PauseMenu::getAndClearAction() {
    MenuAction action = m_currentAction;
    m_currentAction = MenuAction::None;
    return action;
}

void PauseMenu::prepareMapScreen(const sf::Texture& mapTex, sf::Vector2f playerPos,
    sf::Vector2u windowSize,
    const std::map<int, sf::Vector2f>& destinations) {
    if (!mapResourcesInitialized) {
        mapDisplaySprite.setTexture(mapTex);
        mapDisplaySprite.setPosition(0.f, 0.f); // Assuming texture draws from (0,0)
        // You might need to scale mapDisplaySprite if texture is too large/small by default
        // mapDisplaySprite.setScale( appropriate_scale_x, appropriate_scale_y );
        mapResourcesInitialized = true; // Set this early
    }
    // Save player position for centering and marker placement
    playerMapPosition = playerPos;
    missionPoints = destinations;
    destinationMarker.setRadius(20.f);
    destinationMarker.setFillColor(sf::Color::Red);
    destinationMarker.setOrigin(6.f, 6.f);

    mapDisplayView.setSize(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y));
    mapDisplayView.setCenter(playerMapPosition);
    mapDisplayView.zoom(1.0f); // Start with 1:1 zoom, can be adjusted

    playerMarker.setPosition(playerMapPosition);
    // Restart blink timer so the destination marker starts visible when the map opens
    destinationBlinkClock.restart();

    showingMap = true;
    std::cout << "Map screen prepared. Player at: " << playerPos.x << "," << playerPos.y
        << " | Window: " << windowSize.x << "x" << windowSize.y << std::endl;
}

void PauseMenu::prepareStatsScreen(const PlayerGameStats& stats) {
    displayedStats = stats;
    statsDataLoaded = true;
    showingStats = true; // Make stats screen active
    std::cout << "Stats screen prepared." << std::endl;
}

void PauseMenu::updateVolumeDisplayText() {
    float vol = SoundManager::getInstance().getVolume();
    bool muted = SoundManager::getInstance().isMuted();
    std::string volStr;
    if (muted) {
        volStr = "Volume: MUTED";
    }
    else {
        volStr = "Volume: " + std::to_string(static_cast<int>(vol)) + "%";
    }
    volumeLevelText.setString(volStr);

    // Position it - e.g., below the last menu item or at a fixed screen bottom position
    // For now, let's try to position it below the menu items.
    // This requires knowing where the last menu item was drawn.
    // This might be better handled by drawing it at a fixed position in draw() based on window size.
}
