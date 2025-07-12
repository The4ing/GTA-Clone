#pragma once
#include "Vehicle.h"
#include "Pathfinder.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include "Constants.h"

class GameManager;
class Player;
class PatrolZone; // Forward declaration
class Pedestrian;

class PoliceCar : public Vehicle {
public:
    PoliceCar(GameManager& gameManager, const sf::Vector2f& startPosition);
    ~PoliceCar();
    void update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void draw(sf::RenderTarget& target) override;


    void clearOfficerExitRequest();
    void startRetreating(const sf::Vector2f& retreatTarget);
    void setPatrolZone(PatrolZone* zone);
    PatrolZone* getPatrolZone() const;
    sf::Vector2f getPosition() const override;

   
    bool canSeePlayer(const Player& player, const std::vector<std::vector<sf::Vector2f>>& obstacles);
    bool isStatic() const;
    bool attemptRunOverPedestrian(Pedestrian& ped);
    bool isRetreating() const; // Implementation will check internal state
    bool isAmbient() const;
    bool hasOfficerInside() const;

    bool readyForOfficerExit() const;
    void setPosition(const sf::Vector2f& pos) override;
    void setIsStatic(bool isStatic);
    void setIsAmbient(bool isAmbient);
    void setOfficerInside(bool inside);
    bool getNeedsCleanup() const;
    void setNeedsCleanup(bool change);

    bool getPlyrCausedWantedIncrease() const;
    void setPlyrCausedWantedIncrease(bool change);

private:
    enum class CarState { Chasing, AmbientDriving, Retreating };
    CarState m_carState = CarState::AmbientDriving;
    void updateChaseBehavior(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    bool attemptRunOverPlayer(Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    std::vector<sf::Vector2f> getHitboxPolygon() const;

    bool m_isStatic ; // Flag to mark static police cars
    bool m_hasOfficerInside ;
    bool m_isAmbient ;
    bool m_requestOfficerExit;
    bool needsCleanup;
    bool m_playerCausedWantedIncrease;

    int m_playerHitCount ; // Tracks hits specifically against the player
    float m_currentSpeed ;
    int m_bumpCount;
    float m_repathTimer ;
    float m_speed ;
    float m_bumpCooldown ;
    float m_visionDistance ;
    float m_fieldOfViewAngle ; // Degrees


    GameManager& m_gameManager;
    sf::Sound m_sirenSound;

    Pathfinder m_pathfinder;
    sf::Sprite m_sprite;
    sf::Vector2f m_currentTargetPosition;
    std::vector<sf::Vector2f> m_currentPath;
    size_t m_currentPathIndex ;
    PatrolZone* m_assignedZone;


    // Vision parameters (can be different from foot police)
    
};
