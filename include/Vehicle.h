#pragma once
#include "MovingObject.h"
#include "RoadSegment.h"

class Player;

class Vehicle : public MovingObject {
public:
    Vehicle();
    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) override;
    void draw(sf::RenderTarget& target) override;

    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;

    void move(const sf::Vector2f& direction, float dt) override;
    float getSpeed() const override;
    void setTexture(const sf::Texture& texture);
    void setDirectionVec(const std::string& dir);
    void setScale(float scaleX, float scaleY);
    void startTurn(sf::Vector2f from, sf::Vector2f control, sf::Vector2f to);
    //bool shouldTurnTo(const RoadSegment& nextRoad) const;
    void setCurrentRoad(const RoadSegment* road);
    bool isInTurn() const;
    std::string getDirection() const;

    static sf::Vector2f bezier(float t, const sf::Vector2f& P0, const sf::Vector2f& P1, const sf::Vector2f& P2);
    void stop();

    const RoadSegment* getCurrentRoad() const;

    const RoadSegment* currentRoad = nullptr;
    int getCurrentLaneIndex() const { return currentLaneIndex; }
    void setCurrentLaneIndex(int idx) { currentLaneIndex = idx; }
    sf::Vector2f bezierP0, bezierP1, bezierP2; // from, control, to



    void setPreviousRoad(const RoadSegment* road) { previousRoad = road; }
    const RoadSegment* getPreviousRoad() const { return previousRoad; }


    void onCollision(GameObject& other) {};
    void collideWithPresent(Present& present) {};
    void collideWithPlayer(Player& /*player*/) {} // This might need actual implementation laterAdd commentMore actions

    // Player control methods
    void accelerate(float dt);
    void brake(float dt);
    void steerLeft(float dt);
    void steerRight(float dt);

    void setDriver(Player* driver);
    Player* getDriver() const;
    bool hasDriver() const;

    const sf::Sprite& getSprite() const; // Accessor for the sprite

private:
    bool parking;
    Player* m_driver; // Pointer to the player driving this vehicleAdd commentMore actions

    std::string currentDirectionStr; // Used by AI
    sf::Sprite sprite;
    sf::Vector2f position;
    float speed = 70.f;
    float angle = 0.f; // Current angle in degrees, for player steering
    sf::Vector2f directionVec; // Used by AI, and potentially for player if not using angle-based steering

    // Player control parameters
    float accelerationRate = 50.f;
    float decelerationRate = 80.f;
    float maxSpeed = 200.f;
    float reverseSpeed = -50.f; // Max reverse speed
    float turnRate = 90.f; // Degrees per second

    // AI specific members
    bool inTurn = false; // AI Bezier turn
    float bezierT = 0.f;
    float bezierSpeed = 0.0004f;
    const RoadSegment* previousRoad = nullptr;
    int currentLaneIndex = 0;



};
