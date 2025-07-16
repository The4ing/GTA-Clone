#pragma once
#include "Menu.h"
#include <memory>
#include <SFML/Graphics.hpp>
#include <iostream>
#include "Player.h"
#include "HUD.h" 
#include "CarManager.h"
#include "PoliceManager.h"
#include "PedestrianManager.h"
#include "Present.h"
#include "InventoryUI.h"
#include <vector>
#include <string>
#include "BulletPool.h"
#include "PathfindingGrid.h"
#include "Store.h"
#include "QuadTree.h"
#include "PatrolZone.h" 
#include "PauseMenu.h" 
#include "PlayerShooter.h"
#include "Money.h"
#include "Explosion.h"
#include "CarMission.h"
#include "PackageMission.h"
#include <map>
#include "KillMission.h"
#include "SurviveMission.h"
#include "BloodPuddle.h"

enum class GameState {
    Menu,
    Playing,
    Inventory,
    Store,
    Paused, 
    Exiting
};


class GameManager {
public:
    GameManager();
    void addBullet(const sf::Vector2f& startPos, const sf::Vector2f& direction,
        BulletType type = BulletType::Default,
        bool fromPlayer = false,
        bool ignoreBlocked = false);

    void createExplosion(const sf::Vector2f& pos, float radius);
    void createBloodPuddle(const sf::Vector2f& pos);
    void run();

    PathfindingGrid* getPathfindingGrid() const;
    CarManager* getCarManager() const { return carManager.get(); }
    BulletPool& getBulletPool() { return bulletPool; }

    const std::vector<PatrolZone>& getPatrolZones() const;
    const std::vector<RoadSegment>& getRoads() const { return roads; }
    const QuadTree<std::vector<sf::Vector2f>>& getBlockedPolyTree() const { return blockedPolyTree; }
    const sf::View& getGameView() const { return gameView; }

    bool isPositionBlocked(const sf::Vector2f& pos) const;
    
    

private:
    //function to change the game current position
    void setupPatrolZones(); 
    void processEvents();
    void update(float dt);
    void render();
    void startGameFullscreen();
    void setFullscreen(bool fullscreen);
    void loadCollisionRectsFromJSON(const std::string& filename);
    void buildBlockedPolyTree();
    void updatePressStartPosition();
    void renderFrozenGame(sf::RenderTarget& target);
    void displayLoadingScreen(const std::string& message, float initialProgress = 0.0f);
    void loadTasks();
    void startNextTask();

   //variables for the view of the game and managing the diferents  
    QuadTree<std::vector<sf::Vector2f>> blockedPolyTree{ sf::FloatRect(0, 0, 4640, 4672) };
    BulletPool                  bulletPool;
    sf::RenderWindow            window;
    sf::RenderTexture frozenBackgroundTexture;
    sf::View                    gameView;
    sf::View                    m_hudView;
    sf::Sprite mapSprite;
    sf::Sprite frozenBackgroundSprite;
    sf::Texture* mapTexture ;
    GameState                   currentState;
    sf::Clock                   clock;
    sf::Clock missionCompleteClock;
    sf::Clock wastedClock;
    sf::Time                    m_gameTime;
    PauseMenu                   pauseMenu; 
    sf::Text m_pressStartText;
    sf::Text m_taskInstructionText;
    InventoryUI inventoryUI;
    sf::Sound backgroundMusic;
  
    std::map<int, sf::Vector2f> missionDestinations;
   
    //all the main classes mainly opereted 
    std::unique_ptr<Player>     player;
    std::unique_ptr<CarManager> carManager;
    std::unique_ptr<PoliceManager> policeManager;
    std::unique_ptr<PedestrianManager> pedestrianManager;
    std::unique_ptr<PathfindingGrid> pathfindingGrid;
    std::unique_ptr<Menu>       menu;
    std::unique_ptr<HUD>        m_hud;

    //all the storage for all the parts during the running
    std::vector<RoadSegment> roads;
    std::vector<std::vector<sf::Vector2f>> blockedPolygons;
    std::vector<std::unique_ptr<Store>> store;
    std::vector<PatrolZone>     m_patrolZones; 
    std::vector<std::unique_ptr<Present>> presents;
    std::vector<std::unique_ptr<Explosion>> explosions;
    std::vector<std::unique_ptr<BloodPuddle>> bloodPuddles;
    std::vector<std::unique_ptr<Mission>> missions;
    std::vector<std::string> m_taskInstructions;
    
    //variables to maintian changes
    bool showWastedScreen;
    bool showMissionComplete;
    bool wasEscapePressedLastFrame ;
    bool isFullscreen;
    bool m_isAwaitingTaskStart;

    int m_playingFrameCount ; 
    int m_prevWantedLevel;
    int freeNpcHits;
    int freeCopHits;
  
    float overSpeedTime ;

    std::size_t m_currentTaskIndex;
   
   
};
