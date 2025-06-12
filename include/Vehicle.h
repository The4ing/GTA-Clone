#pragma once
#include "MovingObject.h"
#include "RoadSegment.h"

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


    const RoadSegment* getCurrentRoad() const;

    const RoadSegment* currentRoad = nullptr;
    int getCurrentLaneIndex() const { return currentLaneIndex; }
    void setCurrentLaneIndex(int idx) { currentLaneIndex = idx; }
    sf::Vector2f bezierP0, bezierP1, bezierP2; // from, control, to



    void setPreviousRoad(const RoadSegment* road) { previousRoad = road; }
    const RoadSegment* getPreviousRoad() const { return previousRoad; }

private:
    std::string currentDirectionStr;
    sf::Sprite sprite;
    sf::Vector2f position;
    float speed = 70.f;
    sf::Vector2f directionVec;
    bool inTurn = false;
    float bezierT = 0.f;
    float bezierSpeed = 0.0004f;
    const RoadSegment* previousRoad = nullptr;
    int currentLaneIndex = 0;



};

