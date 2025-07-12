#pragma once
#include "Character.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp> 
#include "Constants.h"
#include "Pathfinder.h"
#include "AnimationManager.h"

enum class PoliceState {
    Idle,
    Chasing,
    Shooting,
    MeleeAttacking, 
    BackingUp, 
    Retreating // New state for leaving the screen
};

enum class PoliceWeaponType {
    BATON,
    PISTOL
    // Future: RIFLE, SHOTGUN, etc.
};


class PatrolZone; // Forward declaration
class GameManager;

class Police : public Character {
public:
    Police(GameManager& gameManager, PoliceWeaponType weaponType);

    void setPatrolZone(PatrolZone* zone); 
    PatrolZone* getPatrolZone() const;
    PoliceWeaponType getWeaponType() const;

    void update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void draw(sf::RenderTarget& window);


    void setTargetPosition(const sf::Vector2f& pos);
    void takeDamage(int amount);
    void startRetreating(const sf::Vector2f& retreatTarget);

    bool canSeePlayer(const Player& player, const std::vector<std::vector<sf::Vector2f>>& obstacles);
    bool moveToward(const sf::Vector2f& target, float dt);
    bool isRetreating() const;
    bool isDead() const;
   
  
   
    sf::FloatRect getCollisionBounds() const;
    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;
    bool getMoneyDropped() const;
    void setMoneyDropped(bool change);
    bool getNeedsCleanup() const;
    void setNeedsCleanup(bool change);
    

    float getCollisionRadius() const;
    void onCollision(GameObject& other) {};
    void collideWithPresent(Present& present) {};
    void collideWithPlayer(Player& /*player*/) {}
    void setIsStatic(bool isStatic);
    bool isStatic() const;
 

private:
    void setRandomWanderDestination(const sf::FloatRect& mapBounds);
    bool checkCollision(const sf::Vector2f& currentPos, const sf::Vector2f& nextPos, float radius);
    // bool pointInPolygonWithRadius(const sf::Vector2f& center, float radius, const std::vector<sf::Vector2f>& poly); //  unused for now
    
    float debugPrintTimer ; // for debugging
    float pathFailCooldown ;
    float speed ;
    int health ;
    float detectionRadius;
    float backUpDistance;
    float backedUpSoFar ;
    float pauseTimer ;
    float nextPauseTime ;
    float repathTimer ;
    float fireCooldownTimer ;
    float meleeCooldownTimer;
    float m_visionDistance;
    float m_fieldOfViewAngle;
    float deathTimer ;
    const float deathDuration = 1.f;

    bool m_isStatic;
    bool moneyDropped;
    bool isPaused ;
    bool needsCleanup;
    bool dying;

    sf::Vector2f targetPos; 
    sf::Sprite sprite;
    sf::Vector2f backUpDirection;
    sf::Vector2f wanderDestination;
    sf::Vector2f pathTargetPosition;


    PoliceState state = PoliceState::Idle;
    PoliceWeaponType m_weaponType;
   

    Pathfinder pathfinder;
    std::vector<sf::Vector2f> currentPath;
    size_t currentPathIndex ;
   
   
    GameManager& m_gameManager;

   
    std::unique_ptr<AnimationManager> animationManager;
    int sheetCols ; 
    int sheetRows ;
    int frameWidth;
    int frameHeight;

    void initAnimations();
    void setSpecificFrame(int row, int col); 
    void handleShooting(Player& player, float dt);
    void handleMeleeAttack(Player& player, float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);

    PatrolZone* m_assignedZone = nullptr;
    float radarTimer ;          // סופר את הזמן שהרדאר מוצג
    float radarCooldown ;       // סופר את הזמן בין הפעלות
    bool showRadar;
    sf::Vector2f lastSeenPlayerPosition{}; // Stores player's position when radar activates

      

};


