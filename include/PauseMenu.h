#ifndef PAUSE_MENU_H
#define PAUSE_MENU_H

#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include <map>

class PauseMenu {
public:
    PauseMenu();

    void open();
    void close();
    void update(float dt);
    void draw(sf::RenderTarget& target);
    void handleEvent(const sf::Event& event);
    void prepareMapScreen(const sf::Texture& mapTex, sf::Vector2f playerPos,
        sf::Vector2u windowSize,
        const std::map<int, sf::Vector2f>& destinations);

    bool isOpen() const;

    


    enum class MenuAction { None, RequestNewGame, Resume, RequestOpenMap, RequestOpenStats, Exit};
    MenuAction getAndClearAction();
   
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
    enum class MenuOption { Resume, NewGame, Map, Stats, VolumeUp, VolumeDown, Mute, Exit, Count }; 
    MenuOption selectedOption = MenuOption::Resume;

    sf::Font font;

    sf::CircleShape playerMarker;
    sf::CircleShape destinationMarker;

    sf::View mapDisplayView;

    std::vector<std::string> newGameConfirmOptions = { "Yes", "No" };

    sf::Vector2f playerMapPosition;

    sf::RectangleShape confirmDialogBackground;
    sf::RectangleShape background;

    sf::Text confirmDialogText;
    sf::Text confirmOptionText;
    sf::Text volumeLevelText;
    sf::Text menuText;
    sf::Text titleText;

    sf::Clock escCooldownClock;
    sf::Clock destinationBlinkClock;

    sf::Sprite mapDisplaySprite; 

    std::map<int, sf::Vector2f> missionPoints;
    
    float destinationBlinkInterval;

    int newGameConfirmIndex ; 
    int selectedIndex ; 



    std::vector<std::string> menuItems;
  
    bool statsDataLoaded ;
    bool showingNewGameConfirm ;
    bool mapResourcesInitialized;
    bool m_isOpen ;
    bool showingMap  ;
    bool showingStats ;
    bool wasEscapePressed ;

    void updateVolumeDisplayText();
    void navigateUp();
    void navigateDown();
    void selectCurrent();

};

#endif // PAUSE_MENU_H
