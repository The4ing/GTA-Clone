#ifndef PAUSE_MENU_H
#define PAUSE_MENU_H

#include <vector>
#include <string>
#include <SFML/Graphics.hpp>

class PauseMenu {
public:
    PauseMenu();

    void open();
    void close();
    bool isOpen() const;

    void update(float dt);
    void draw(sf::RenderTarget& target);
    void handleEvent(const sf::Event& event);

private:
    enum class MenuOption { Resume, NewGame, Map, Stats, VolumeUp, VolumeDown, Mute, Exit, Count }; // Added Count for easier iteration if needed
    std::vector<std::string> menuItems;
    MenuOption selectedOption = MenuOption::Resume; // Store as enum for type safety
    int selectedIndex = 0; // Kept for easier navigation with % operator, will sync with selectedOption

    bool m_isOpen = false;
    bool showingMap = false;
    bool showingStats = false;
    bool wasEscapePressed = false;

    sf::Font font; // Consider moving font loading to ResourceManager if available
    sf::Text menuText;
    sf::RectangleShape background;
    sf::Text titleText;

    void navigateUp();
    void navigateDown();
    void selectCurrent();

public: // Public for GameManager to access
    enum class MenuAction { None, RequestNewGame, Resume, RequestOpenMap, RequestOpenStats, Exit};
    MenuAction getAndClearAction();
    void prepareMapScreen(const sf::Texture& mapTex, sf::Vector2f playerPos, sf::Vector2u windowSize);

    struct PlayerGameStats { // Renamed to avoid conflict
        sf::Time gameTime = sf::Time::Zero;
        int kills = 0;
        int money = 0;
        int wantedLevel = 0;
        std::vector<std::string> collectedWeapons;
    };
    void prepareStatsScreen(const PlayerGameStats& stats);

private:
    MenuAction m_currentAction = MenuAction::None;
    PlayerGameStats displayedStats; 
    bool statsDataLoaded = false;   
    sf::Clock escCooldownClock;

    // Map related members
    sf::Sprite mapDisplaySprite; 

    // New Game confirmation
    bool showingNewGameConfirm = false;
    int newGameConfirmIndex = 0; // 0 for Yes, 1 for No
    std::vector<std::string> newGameConfirmOptions = {"Yes", "No"};
    sf::RectangleShape confirmDialogBackground;
    sf::Text confirmDialogText;
    sf::Text confirmOptionText;

    // Volume display
    sf::Text volumeLevelText;
    void updateVolumeDisplayText(); // Helper to update the volume text

    sf::View mapDisplayView;
    sf::CircleShape playerMarker;
    sf::Vector2f playerMapPosition;
    bool mapResourcesInitialized = false;
};

#endif // PAUSE_MENU_H
