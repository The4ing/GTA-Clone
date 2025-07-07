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

    bool canSeePlayer(const Player& player, const std::vector<std::vector<sf::Vector2f>>& obstacles);
    void update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    bool moveToward(const sf::Vector2f& target, float dt);
    void draw(sf::RenderTarget& window);
    void startRetreating(const sf::Vector2f& retreatTarget); 
    bool isRetreating() const { return state == PoliceState::Retreating; }
    bool needsCleanup = false; // Flag to mark for removal after retreating
    bool dying = false;
    float deathTimer = 0.f;
    const float deathDuration = 1.f;
    void takeDamage(int amount);
    bool isDead() const;

    float getCollisionRadius() const;
    sf::FloatRect getCollisionBounds() const;
    void setTargetPosition(const sf::Vector2f& pos); 

    sf::Vector2f getPosition() const override {
        return sprite.getPosition();
    }
    void setPosition(const sf::Vector2f& pos) override {
        sprite.setPosition(pos);
    }

    PoliceWeaponType getWeaponType() const { return m_weaponType; } 

    void onCollision(GameObject& other) {};
    void collideWithPresent(Present& present) {};
    void collideWithPlayer(Player& /*player*/) {}
    void setIsStatic(bool isStatic) { m_isStatic = isStatic; }
    bool isStatic() const { return m_isStatic; }
 

private:
    void setRandomWanderDestination(const sf::FloatRect& mapBounds);
    bool checkCollision(const sf::Vector2f& currentPos, const sf::Vector2f& nextPos, float radius);
    // bool pointInPolygonWithRadius(const sf::Vector2f& center, float radius, const std::vector<sf::Vector2f>& poly); //  unused for now
    float debugPrintTimer = 0.f; // for debugging
    float pathFailCooldown = 0.f;

    sf::Vector2f targetPos; 
    sf::Sprite sprite;
    float speed = 40.f; 
    float detectionRadius = 150.f; 
    int health = 100;
    PoliceState state = PoliceState::Idle;
    PoliceWeaponType m_weaponType;
    float backUpDistance = 30.f;
    float backedUpSoFar = 0.f;
    sf::Vector2f backUpDirection;
    sf::Vector2f wanderDestination;
    float pauseTimer = 0.f;
    float nextPauseTime = 0.f;
    bool isPaused = false;

    Pathfinder pathfinder;
    std::vector<sf::Vector2f> currentPath;
    size_t currentPathIndex = 0;
    float repathTimer = 0.f;
    sf::Vector2f pathTargetPosition; 

    GameManager& m_gameManager;


    float fireCooldownTimer = 0.f;
    const float PISTOL_FIRE_RATE = 1.5f; 
    const float PISTOL_SHOOTING_RANGE = 200.f; 
    const float PISTOL_LINE_OF_SIGHT_RANGE = 250.f; 


    float meleeCooldownTimer = 0.f;
    const float BATON_MELEE_RATE = 1.0f; 
    const float BATON_MELEE_RANGE = 40.f; 
    const int BATON_DAMAGE = 10;

    std::unique_ptr<AnimationManager> animationManager;
    int sheetCols = 10; 
    int sheetRows = 10;
    int frameWidth;
    int frameHeight;

    void initAnimations();
    void setSpecificFrame(int row, int col); 
    void handleShooting(Player& player, float dt);
    void handleMeleeAttack(Player& player, float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);

    PatrolZone* m_assignedZone = nullptr;
    float radarTimer = 0.f;          // סופר את הזמן שהרדאר מוצג
    float radarCooldown = 0.f;       // סופר את הזמן בין הפעלות
    bool showRadar = false;
    sf::Vector2f lastSeenPlayerPosition{}; // Stores player's position when radar activates

        // Vision parameters
    float m_visionDistance = 200.f;
    float m_fieldOfViewAngle = 120.f; // Degrees
    bool m_isStatic = false; // Flag to mark static police unitsAdd commentMore actions

};


