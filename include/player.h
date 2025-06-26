#pragma once
#include "Character.h"
#include <SFML/Graphics.hpp>
#include "Constants.h"
#include "Inventory.h"
#include "AnimationManager.h"
class Vehicle;

class Player : public Character {
public:
    Player();

    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition() const;
    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void draw(sf::RenderTarget& window);
    sf::FloatRect getCollisionBounds(const sf::Vector2f& offset = {}) const;
    sf::Vector2f getCenter() const;
    float getCollisionRadius() const;
    void takeDamage(int amount) ;

    // HUD Data Getters
    int getMoney() const { return m_money; }
    int getHealth() const { return m_health; }
    int getArmor() const { return m_armor; }
    std::string getCurrentWeaponName() const { return m_currentWeaponName; }
    int getCurrentAmmo(const std::string& name) const {
          return WeaponsAmmo.at(name).Ammo;  // throws std::out_of_range if not found
      }
    int getMaxAmmo() const { return m_maxWeaponAmmo; }
    int getWantedLevel() const { return m_wantedLevel; }

    // HUD Data Setters (for testing/game logic)
    void setMoney(int money) { m_money = money; }
    void setHealth(int health) { m_health = std::max(0, health); } // Prevent negative health
    void setArmor(int armor) { m_armor = std::max(0, armor); }   // Prevent negative armor
    void setWantedLevel(int level) { m_wantedLevel = std::max(0, std::min(level, 6)); } // Clamp 0-6
    void setCurrentWeapon(const std::string& name, CurrentWepapon Weapon , int maxAmmo) {
        m_currentWeaponName = name;
        m_currentWeaponAmmo = Weapon;
        m_maxWeaponAmmo = maxAmmo;
    }
    
    void onCollision(GameObject& other) ;
    void collideWithPresent(Present& present);
    void collideWithPlayer(Player& /*player*/)  {} 


    Inventory& getInventory();
    const Inventory& getInventory() const;

    void heal(int amount);
    void increaseSpeed(); 
    void AddAmmo();
    void AddWeapon(const std::string name);
    



    // Vehicle interactionAdd commentMore actionsAdd commentMore actions
    void enterVehicle(Vehicle* vehicle);
    void exitVehicle();
    Vehicle* getCurrentVehicle() const;
    bool isInVehicle() const;

private:
    Vehicle* m_currentVehicle;
    void setSpecificFrame(int row, int col);
    void playAnimation(const std::string& animName, bool loop = true, bool pingpong = false);
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
    bool wasShooting = false;
    bool isFinishingShootAnim = false;

    // HUD-related members
    int m_money;
    int m_health;
    int m_armor;
    std::string m_currentWeaponName;
    CurrentWepapon m_currentWeaponAmmo; // -1 for melee/infinite
    int m_maxWeaponAmmo;
    int m_wantedLevel;

    Inventory inventory;
    int Bullets = 0; 
    float speedBoostTimer = 0.f;
    std::string currentAnimationName;
    std::unique_ptr<AnimationManager> animationManager;
      std::unordered_map<std::string, AmmoSetting> WeaponsAmmo;
};
