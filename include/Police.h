#pragma once

#include "Character.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp> // Added
#include "Constants.h"
// #include "PathPlanner.h" // Original path planner, to be replaced or unused
#include "Pathfinder.h"    // Added for A*
#include "QuadTree.h"      // Assuming QuadTree might be used for something else

enum class PoliceState {
    Idle,
    Chasing
};

class Police : public Character {
public:
    Police(sf::Vector2f target);

    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void moveToward(const sf::Vector2f& target, float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void draw(sf::RenderWindow& window);

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

private:
    void setRandomWanderDestination(const sf::FloatRect& mapBounds);
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
    float wanderTimer = 0.f;
    sf::Vector2f wanderDestination;

    Pathfinder pathfinder; // Added Pathfinder member

    int currentFrame = 0;
    float animationTimer = 0.f;
    float animationSpeed = 0.13f;

    // These were already uncommented, no change needed here
    std::vector<sf::Vector2f> currentPath;
    size_t currentPathIndex = 0;
    float repathTimer = 0.f;

public: // Made constant public for access in .cpp or for other classes if needed
    static constexpr float PATHFINDING_GRID_SIZE = 32.0f;
};
