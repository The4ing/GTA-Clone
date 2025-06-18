#pragma once
#include "Character.h"
#include <SFML/Graphics.hpp>
#include "Constants.h"
#include "Inventory.h"

class Player : public Character {
public:
    Player();

    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition() const;
    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void draw(sf::RenderWindow& window);
    sf::FloatRect getCollisionBounds(const sf::Vector2f& offset = {}) const;
    sf::Vector2f getCenter() const;
    float getCollisionRadius() const;
    void takeDamage(int amount) ;
    
    void onCollision(GameObject& other) ;
    void collideWithPresent(Present& present);
    void collideWithPlayer(Player& /*player*/)  {} 


    Inventory& getInventory();
    const Inventory& getInventory() const;

    void heal(int amount);

private:

    sf::Sprite  sprite;
    float       speed = 250.f;
    int         frameWidth;
    int         frameHeight;
    int         currentFrame;
    int         sheetCols;
    int         sheetRows;
    float       animTimer;
    float       animDelay;
    sf::Vector2f position;


    int health = 100;
    Inventory inventory;
};
