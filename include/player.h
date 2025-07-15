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

    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition() const;
    float getRotation() const;
    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    void draw(sf::RenderTarget& window);
    sf::FloatRect getCollisionBounds(const sf::Vector2f& offset = {}) const;
    sf::Vector2f getCenter() const;
    float getCollisionRadius() const;
    void takeDamage(int amount);

    // HUD Data Getters
    int getMoney() const;
    int getHealth() const;
    int getArmor() const;
    std::string getCurrentWeaponName() const;
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
    Vehicle* getCurrentVehicle() const;
    bool isInVehicle() const;
    int getKills() const { return killCount; }

    void incrementKills() { ++killCount; } 

    PlayerShooter& getShooter();
    GameManager& getGameManager();
    void applyKnockback(const sf::Vector2f& velocity, float duration);

private:
    Vehicle* m_currentVehicle;
    void setSpecificFrame(int row, int col);
    void playAnimation(const std::string& animName, bool loop = true, bool pingpong = false, bool forceRestart = false);
    sf::CircleShape getCollisionCircle() const;


    sf::Sprite  sprite;
    float       speed = 50.f;
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
    int killCount = 0;
    bool m_isDead = false;
    int m_money;
    int m_health;
    int m_armor;
    std::string m_currentWeaponName;
    CurrentWepapon m_currentWeaponAmmo;
    int m_maxWeaponAmmo;
    int m_wantedLevel;
    std::unique_ptr<PlayerShooter> m_shooter;

    Inventory inventory;
    int Bullets = 0;
    float speedBoostTimer = 0.f;
    sf::Vector2f m_knockbackVelocity = { 0.f, 0.f };
    float m_knockbackTimer = 0.f;
    std::string currentAnimationName;
    std::unique_ptr<AnimationManager> animationManager;
    std::unordered_map<std::string, AmmoSetting> WeaponsAmmo;
    GameManager& m_gameManager;  // ????? ????
};
