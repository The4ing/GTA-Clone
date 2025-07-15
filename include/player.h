#pragma once
#include "Character.h"
#include <SFML/Graphics.hpp>
#include "Constants.h"
#include "Inventory.h"
#include "AnimationManager.h"
#include "Bullet.h"
#include <memory>

class PlayerShooter;

class Vehicle;
class GameManager; // Forward declaration ????

class Player : public Character {
public:
    Player(GameManager& gameManager);
    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void draw(sf::RenderTarget& window);
    void setPosition(const sf::Vector2f& pos);

    sf::Vector2f getPosition() const;
    sf::FloatRect getCollisionBounds(const sf::Vector2f& offset = {}) const;
    sf::Vector2f getCenter() const;
    sf::CircleShape getCollisionCircle() const;
    std::string getCurrentWeaponName() const;
    Vehicle* getCurrentVehicle() const;
    PlayerShooter& getShooter();
    GameManager& getGameManager();

    float getRotation() const;
    void takeDamage(int amount);
    float getCollisionRadius() const;
    

    // HUD Data Getters
    int getMoney() const;
    int getHealth() const;
    int getArmor() const;
    int getCurrentAmmo(const std::string& name) const;
    int getMaxAmmo() const;
    int getWantedLevel() const;

    bool consumeAmmo(const std::string& name);

    // HUD Data Setters
    void setMoney(int money);
    void setHealth(int health);
    void setArmor(int armor);
    void setWantedLevel(int level);
    void playThrowAnimation();
    void setCurrentWeapon(const std::string& name, int maxAmmo);


    void onCollision(GameObject& other);
    void collideWithPresent(Present& present);
    void collideWithPlayer(Player& /*player*/);

    Inventory& getInventory();
    const Inventory& getInventory() const;

    void heal(int amount);
    void increaseSpeed(bool ShopItem);
    void AddAmmo();
    bool tryBuyAmmo(const std::string& weaponName, int amountToAdd, int price);
    void decreaseMoney(int priceItem);
    void AddWeapon(const std::string name);

    void enterVehicle(Vehicle* vehicle);
    void exitVehicle();
   
    bool isInVehicle() const;
    int getKills() const;

    void incrementKills();
    void incrementNpcKills();
    void incrementCopKills();
    int getNpcKills() const;
    int getCopKills() const;

  
    void applyKnockback(const sf::Vector2f& velocity, float duration);
    void resetAfterDeath();
    void resetMissionKills();

private:
    Vehicle* m_currentVehicle;
    void setSpecificFrame(int row, int col);
    void playAnimation(const std::string& animName, bool loop = true, bool pingpong = false, bool forceRestart = false);


    sf::Sprite  sprite;

    float       animTimer;
    float       animDelay;
    float       speed ;
    float m_knockbackTimer ;
    float speedBoostTimer ;

    int         frameWidth;
    int         frameHeight;
    int         currentFrame;
    int         sheetCols;
    int         sheetRows;
    int m_money;
    int m_health;
    int m_armor;
    int killCount ;
    int Bullets ;
    int m_maxWeaponAmmo;
    int m_wantedLevel;
    int npcKills ;
    int copKills;

    sf::Vector2f position;
    bool wasShooting ;
    bool isFinishingShootAnim ;
    bool m_isDead ;
    
    std::string m_currentWeaponName;
    std::string currentAnimationName;

    std::unique_ptr<PlayerShooter> m_shooter;

    Inventory inventory;
   

    sf::Vector2f m_knockbackVelocity ;
   
   
    std::unique_ptr<AnimationManager> animationManager;
    std::unordered_map<std::string, AmmoSetting> WeaponsAmmo;
    GameManager& m_gameManager;  
};
