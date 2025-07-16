#pragma once
#include "MovingObject.h"
#include "RoadSegment.h"
#include <vector>
#include "Constants.h"
class Player;

class Vehicle : public MovingObject {
public:
    Vehicle();
    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) override;
    void draw(sf::RenderTarget& target) override;
    void move(const sf::Vector2f& direction, float dt) override;
    void setTexture(const sf::Texture& texture);
    void setDirectionVec(const std::string& dir);
    void setScale(float scaleX, float scaleY);
    void startTurn(sf::Vector2f from, sf::Vector2f control, sf::Vector2f to);
    void setPosition(const sf::Vector2f& pos) override;
    void setCurrentRoad(const RoadSegment* road);
    void setTextureRect(const sf::IntRect& rect);
    void stop();
    void stopForSeconds(float seconds);
    void setCurrentLaneIndex(int idx);
    void setPreviousRoad(const RoadSegment* road);
    void setDestroyed(bool value);
    // Player control methods
    void accelerate(float dt);
    void brake(float dt);
    void steerLeft(float dt);
    void steerRight(float dt);
    void setDriver(Player* driver);
    void setActive(bool value);

    float getSpeed() const override;
    int getCurrentLaneIndex() const;
   
   
    bool isInTurn() const;
    bool isActive() const;
    bool isDestroyed() const;
    bool hasDriver() const;

    std::string getDirection() const;
   

    static sf::Vector2f bezier(float t, const sf::Vector2f& P0, const sf::Vector2f& P1, const sf::Vector2f& P2);
   
    const RoadSegment* getCurrentRoad() const;
    const RoadSegment* currentRoad = nullptr;
    const RoadSegment* getPreviousRoad() const;
    const sf::Sprite& getSprite() const; // Accessor for the sprite

   
    sf::Vector2f getPosition() const override;
   


    void onCollision(GameObject& other) {};
    void collideWithPresent(Present& present) {};
    void collideWithPlayer(Player& /*player*/) {} // This might need actual implementation laterAdd commentMore actions

 
    Player* getDriver() const;
   
    sf::Sprite& getSprite();
    std::vector<sf::Vector2f> getHitboxPolygon() const;
  
   
    // Indicates if the derived class handles its own AI rotation, preventing generic Vehicle AI rotation.
    virtual bool handlesOwnAIRotation() const;

protected:
    bool destroyed = false;

private:
    bool parking;
    Player* m_driver; 

    std::string currentDirectionStr; 
    sf::Sprite sprite;
    sf::Vector2f position;
    sf::Vector2f directionVec;

    float bezierT ;
    float bezierSpeed ;
    float speed ;
    float angle ; 
    float accelerationRate ;
    float decelerationRate ;
    float maxSpeed ;
    float reverseSpeed ;
    float turnRate ; 
    float stopTimer ;
  
    bool inTurn = false; 
    bool active = false;

    const RoadSegment* previousRoad = nullptr;
    int currentLaneIndex = 0;
    
  
    sf::Vector2f bezierP0, bezierP1, bezierP2;

};
