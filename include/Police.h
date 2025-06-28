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
    BackingUp
};

class GameManager;

class Police : public Character {
public:
    Police(GameManager& gameManager);

    void update(float dt, const sf::Vector2f& playerPosition, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons); 
    bool moveToward(const sf::Vector2f& target, float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons); 
    void draw(sf::RenderTarget& window);

    void takeDamage(int amount);
    bool isDead() const;

    float getCollisionRadius() const;
    void setTargetPosition(const sf::Vector2f& pos);

    sf::Vector2f getPosition() const override {
        return sprite.getPosition();
    }
    void setPosition(const sf::Vector2f& pos) override {
        sprite.setPosition(pos);
    }

    void onCollision(GameObject& other) {};
    void collideWithPresent(Present& present) {};
    void collideWithPlayer(Player& /*player*/) {}
    static constexpr float PATHFINDING_GRID_SIZE = 32.0f;

private:
    void setRandomWanderDestination(const sf::FloatRect& mapBounds);
    bool checkCollision(const sf::Vector2f& currentPos, const sf::Vector2f& nextPos, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons, float radius);
    bool pointInPolygonWithRadius(const sf::Vector2f& center, float radius, const std::vector<sf::Vector2f>& poly);
    float debugPrintTimer = 0.f;
    float pathFailCooldown = 0.f;

    sf::Vector2f targetPos;
    
    int framesPerRow = 6;
    sf::Sprite sprite;
    float speed = 40.f;
    float detectionRadius = 100.f;
    int health = 100;
    PoliceState state = PoliceState::Idle;
    float backUpDistance = 30.f; 
    float backedUpSoFar = 0.f;   
    sf::Vector2f backUpDirection;  
    float wanderTimer = 0.f;
    sf::Vector2f wanderDestination;
    float pauseTimer = 0.f;      // כמה זמן נשאר לעצור עכשיוAdd commentMore actions
    float nextPauseTime = 0.f;   // מתי לעצור בפעם הבאה (בין 30 ל-60 שניות)
    bool isPaused = false;

    int currentFrame = 0;
    float animationTimer = 0.f;
    float animationSpeed;
    Pathfinder pathfinder; // Added Pathfinder member
    std::vector<sf::Vector2f> currentPath;
    size_t currentPathIndex = 0;
    float repathTimer = 0.f;
    sf::Vector2f pathTargetPosition;

    GameManager& m_gameManager; // Reference to GameManager for shootingAdd commentMore actions
    float fireCooldownTimer = 0.f;
    const float fireRate = 1.5f; // Seconds between shots
    const float shootingRange = 200.f; // Max distance to shoot
    const float lineOfSightRange = 250.f; // Max distance to detect/see player for shooting state

    std::unique_ptr<AnimationManager> animationManager; 
    int sheetCols = 10;
    int sheetRows = 10;
    int frameWidth;
    int frameHeight;

    void initAnimations();
    void setSpecificFrame(int row, int col);
    void handleShooting(const sf::Vector2f& playerPosition, float dt);
};
