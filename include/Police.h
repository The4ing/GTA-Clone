#pragma once

#include "Character.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp> 
#include "Constants.h"
//#include "PathPlanner.h" 
#include "Pathfinder.h"  
#include  "QuadTree.h"

enum class PoliceState {
    Idle,
    Chasing,
    BackingUp
};


class Police : public Character {
public:
    Police(sf::Vector2f target);

    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
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
    int frameWidth;
    int frameHeight;
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

    int currentFrame = 0;
    float animationTimer = 0.f;
    float animationSpeed = 0.13f;
    Pathfinder pathfinder; // Added Pathfinder member
    std::vector<sf::Vector2f> currentPath;
    size_t currentPathIndex = 0;
    float repathTimer = 0.f;
};
