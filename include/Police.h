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
    MeleeAttacking, // Added for baton attacks
    BackingUp
};

enum class PoliceWeaponType {
    BATON,
    PISTOL
    // Future: RIFLE, SHOTGUN, etc.
};



class GameManager;

class Police : public Character {
public:
    // Modified constructor to accept weapon type
    Police(GameManager& gameManager, PoliceWeaponType weaponType);

    // Update now takes Player reference for melee attacks
    void update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    bool moveToward(const sf::Vector2f& target, float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void draw(sf::RenderTarget& window);

    void takeDamage(int amount);
    bool isDead() const;

    float getCollisionRadius() const;
    void setTargetPosition(const sf::Vector2f& pos); // Keep this to update internal targetPos

    sf::Vector2f getPosition() const override {
        return sprite.getPosition();
    }
    void setPosition(const sf::Vector2f& pos) override {
        sprite.setPosition(pos);
    }

    PoliceWeaponType getWeaponType() const { return m_weaponType; } // Getter for weapon type

    void onCollision(GameObject& other) {};
    void collideWithPresent(Present& present) {};
    void collideWithPlayer(Player& /*player*/) {}
 

private:
    void setRandomWanderDestination(const sf::FloatRect& mapBounds);
    bool checkCollision(const sf::Vector2f& currentPos, const sf::Vector2f& nextPos, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons, float radius);
    // bool pointInPolygonWithRadius(const sf::Vector2f& center, float radius, const std::vector<sf::Vector2f>& poly); // This seems unused, consider removing if confirmed
    float debugPrintTimer = 0.f; // Keep if used for debugging
    float pathFailCooldown = 0.f;

    sf::Vector2f targetPos; // Player's position, updated by setTargetPosition

    // int framesPerRow = 6; // Likely managed by AnimationManager now
    sf::Sprite sprite;
    float speed = 40.f; // Base speed, can be adjusted
    float detectionRadius = 150.f; // General detection radius
    int health = 100;
    PoliceState state = PoliceState::Idle;
    PoliceWeaponType m_weaponType;

    // Movement / State specific
    float backUpDistance = 30.f;
    float backedUpSoFar = 0.f;
    sf::Vector2f backUpDirection;
    sf::Vector2f wanderDestination;
    float pauseTimer = 0.f;
    float nextPauseTime = 0.f;
    bool isPaused = false;

    // Animation related (Some might be handled by AnimationManager)
    // int currentFrame = 0; // Likely managed by AnimationManager
    // float animationTimer = 0.f; // Likely managed by AnimationManager
    // float animationSpeed; // Likely managed by AnimationManager

    Pathfinder pathfinder;
    std::vector<sf::Vector2f> currentPath;
    size_t currentPathIndex = 0;
    float repathTimer = 0.f;
    sf::Vector2f pathTargetPosition; // Position the current path is leading to

    GameManager& m_gameManager;

    // Weapon specific parameters
    // Pistol
    float fireCooldownTimer = 0.f;
    const float PISTOL_FIRE_RATE = 1.5f; // Seconds between shots
    const float PISTOL_SHOOTING_RANGE = 200.f; // Max distance to shoot
    const float PISTOL_LINE_OF_SIGHT_RANGE = 250.f; // Max distance to detect/see player for shooting state

    // Baton
    float meleeCooldownTimer = 0.f;
    const float BATON_MELEE_RATE = 1.0f; // Seconds between attacks
    const float BATON_MELEE_RANGE = 40.f; // Max distance for baton attack (adjust based on sprite size)
    const int BATON_DAMAGE = 10;

    std::unique_ptr<AnimationManager> animationManager;
    int sheetCols = 10; // Sprite sheet dimensions, ensure these are correct
    int sheetRows = 10;
    int frameWidth;
    int frameHeight;

    void initAnimations();
    void setSpecificFrame(int row, int col); // Could be part of AnimationManager or for specific non-animated states
    void handleShooting(Player& player, float dt); // Pass player for LOS checks etc.
    void handleMeleeAttack(Player& player, float dt); // New handler for melee
};


