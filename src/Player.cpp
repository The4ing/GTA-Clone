#include "Player.h"
#include "GameManager.h" // פה כן כוללים את ההגדרה המלאה של GameManager
#include <SFML/Window/Keyboard.hpp>
#include "ResourceManager.h"
#include <cmath>
#include <algorithm>
#include "CollisionUtils.h"
#include <iostream>
#include "Present.h"
#include "Vehicle.h"

Player::Player(GameManager& gameManager) // Modified constructor
    : m_gameManager(gameManager), // Store GameManager reference
    m_currentVehicle(nullptr), frameWidth(0), frameHeight(0), currentFrame(0),
    sheetCols(12), sheetRows(12), animTimer(0.f), animDelay(0.1f),
    m_money(PlayerMoney), m_health(MaxHealth), m_armor(100),
    m_currentWeaponName("Fists"), m_maxWeaponAmmo(0),
    m_wantedLevel(3)
{
    sf::Texture& texture = ResourceManager::getInstance().getTexture("player");
    sprite.setTexture(texture);

    int sheetW = texture.getSize().x;
    int sheetH = texture.getSize().y;
    frameWidth = sheetW / sheetCols;
    frameHeight = sheetH / sheetRows;

    sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
    sprite.setScale(0.25f, 0.25f);
    sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));

    position = { 100.f, 100.f };
    sprite.setPosition(position);
    sprite.setScale(0.07, 0.07);

    animationManager = std::make_unique<AnimationManager>(sprite, frameWidth, frameHeight, sheetCols, sheetRows);
    animationManager->initAnimations();

    WeaponsAmmo = {
    { "Fists",   AmmoSetting{0, 0} },        
    { "Pistol",  AmmoSetting{12, 60} },      
    { "Rifle",   AmmoSetting{30, 180} },     
    { "Minigun", AmmoSetting{100, 1000} },   
    { "Bazooka", AmmoSetting{1, 5} },        
    { "Knife",   AmmoSetting{0, 0} },        
    { "Grenade", AmmoSetting{3, 10} }        
    };
}

void Player::setPosition(const sf::Vector2f& pos) {
    sprite.setPosition(pos);
}

sf::Vector2f Player::getPosition() const {
    return sprite.getPosition();
}



void Player::takeDamage(int amount)
{
    if (m_armor > 0) {
        int armorDamage = std::min(m_armor, amount);
        m_armor -= armorDamage;
        amount -= armorDamage;
    }
    if (amount > 0) {
        m_health -= amount;
        if (m_health < 0) m_health = 0;
    }
}



int Player::getCurrentAmmo(const std::string& name) const {
    auto it = WeaponsAmmo.find(name);
    if (it != WeaponsAmmo.end())
        return it->second.Ammo;
    return 0; // או ערך ברירת מחדל מתאים
}


void Player::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    if (m_currentVehicle) {
        // שחקן ברכב - מעביר שליטה לרכב
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            m_currentVehicle->accelerate(dt);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            m_currentVehicle->brake(dt);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            m_currentVehicle->steerLeft(dt);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            m_currentVehicle->steerRight(dt);

        // סנכרון מיקום השחקן עם הרכב
        setPosition(m_currentVehicle->getPosition());
    }
    else {
        // שחקן הולך ברגל
        sf::Vector2f movement(0.f, 0.f);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            movement.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            movement.x += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            movement.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            movement.y += 1.f;

        bool isMoving = (movement.x != 0.f || movement.y != 0.f);
        bool isShooting = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
        bool isHit = false;
        bool isDead = false;

        if (speedBoostTimer > 0.f)
            speedBoostTimer -= dt;
        else
            speed = BasicSpeed;

        if (isMoving) {
            float len = std::sqrt(movement.x * movement.x + movement.y * movement.y);
            if (len != 0.f)
                movement /= len;

            float angle = std::atan2(movement.y, movement.x) * 180.f / 3.14159f + 90.f;
            sprite.setRotation(angle);

            sf::Vector2f nextPos = sprite.getPosition() + movement * speed * dt;

            bool collision = false;
            for (const auto& poly : blockedPolygons) {
                if (CollisionUtils::pointInPolygon(nextPos, poly)) {
                    collision = true;
                    break;
                }
            }

            if (!collision) {
                sprite.move(movement * speed * dt);
            }
        }

        bool shootingAnimPlaying = false;
        if (!currentAnimationName.empty()) {
            bool isShootAnim = currentAnimationName.find("Throw") != std::string::npos ||
                currentAnimationName.find("Shoot") != std::string::npos ||
                currentAnimationName.find("Attack") != std::string::npos;
            shootingAnimPlaying = isShootAnim && !animationManager->isAnimationFinished();
        }

        if (isDead) {
            playAnimation("HurtDie");
        }
        else if (isHit) {
            playAnimation("Hurt");
        }
        else {
            if (isShooting && !shootingAnimPlaying) {
                if (m_currentWeaponName == "Fists") {
                    playAnimation("BatAttack", false);
                }
                else if (m_currentWeaponName == "Pistol") {
                    playAnimation("PistolShoot", false);
                    SoundManager::getInstance().playSound("gunshot");
                    // Create and add bulletAdd commentMore actions
                    float angleRad = (sprite.getRotation() - 90.f) * (3.14159f / 180.f); // Convert angle to radians, adjust for sprite orientation
                    sf::Vector2f bulletDir(std::cos(angleRad), std::sin(angleRad));
                    m_gameManager.addBullet(getCenter(), bulletDir);
                }
                else if (m_currentWeaponName == "Rifle") {
                    playAnimation("RifleShoot", false);
                    SoundManager::getInstance().playSound("rifleShot");
                    float angleRad = (sprite.getRotation() - 90.f) * (3.14159f / 180.f); 
                    sf::Vector2f bulletDir(std::cos(angleRad), std::sin(angleRad));
                    m_gameManager.addBullet(getCenter(), bulletDir);

                }
                else if (m_currentWeaponName == "Minigun") {
                    playAnimation("MinigunShoot", false);
                    SoundManager::getInstance().playSound("minigunShot");
                    float angleRad = (sprite.getRotation() - 90.f) * (3.14159f / 180.f);
                    sf::Vector2f bulletDir(std::cos(angleRad), std::sin(angleRad));
                    m_gameManager.addBullet(getCenter(), bulletDir);
                }
                else if (m_currentWeaponName == "Bazooka") {
                    playAnimation("BazookaShoot", false);
                    SoundManager::getInstance().playSound("RPGshot");
                    float angleRad = (sprite.getRotation() - 90.f) * (3.14159f / 180.f);
                        sf::Vector2f bulletDir(std::cos(angleRad), std::sin(angleRad));
                        m_gameManager.addBullet(getCenter(), bulletDir); // Consider a different bullet type for RPG later
                }
                else if (m_currentWeaponName == "Knife") {
                    playAnimation("KnifeAttack", false);
                    SoundManager::getInstance().playSound("KnifeAttack");
                }
                else if (m_currentWeaponName == "Grenade") {
                    playAnimation("ThrowGrenade", false);
                    SoundManager::getInstance().playSound("ThrowGrenade");
                }
            }
            else if (shootingAnimPlaying) {
                // המשך ניגון של אנימציית ירי - לא עושים כלום
            }
            else {
                if (m_currentWeaponName == "Fists") {
                    if (isMoving)
                        playAnimation("Idle_NoWeapon");
                    else {
                        setSpecificFrame(0, 0);
                        return;
                    }
                }
                else if (m_currentWeaponName == "Pistol") {
                    if (isMoving)
                        playAnimation("PistolWalk");
                    else {
                        setSpecificFrame(6, 10);
                        return;
                    }
                }
                else if (m_currentWeaponName == "Rifle") {
                    if (isMoving)
                        playAnimation("RifleWalk");
                    else {
                        setSpecificFrame(5, 5);
                        return;
                    }
                }
                else if (m_currentWeaponName == "Minigun") {
                    if (isMoving)
                        playAnimation("MinigunWalk");
                    else {
                        setSpecificFrame(8, 1);
                        return;
                    }
                }
                else if (m_currentWeaponName == "Bazooka") {
                    if (isMoving)
                        playAnimation("BazookaWalk");
                    else {
                        setSpecificFrame(9, 9);
                        return;
                    }
                }
                else if (m_currentWeaponName == "Knife") {
                    playAnimation("Idle_NoWeapon");
                }
                else if (m_currentWeaponName == "Grenade") {
                    playAnimation("Idle_NoWeapon");
                }
            }
        }
    }

    animationManager->update(dt);
}






void Player::draw(sf::RenderTarget& window) {
    if (m_currentVehicle) {
            // Optionally, don't draw player sprite, or draw a different "in car" sprite
            // For now, we'll just not draw the player if they are in a vehicle,
            // assuming the vehicle itself will be drawn by CarManager.
            return;
    }
    sf::CircleShape circle(getCollisionRadius());
    circle.setOrigin(getCollisionRadius(), getCollisionRadius());
    circle.setPosition(getCenter());
    circle.setFillColor(sf::Color::Transparent);
    circle.setOutlineColor(sf::Color::Blue);
    circle.setOutlineThickness(1.f);
    window.draw(circle);


    window.draw(sprite);
}

sf::FloatRect Player::getCollisionBounds(const sf::Vector2f& offset) const {
    // Get the center position of the sprite (already scaled)
    if (m_currentVehicle) {
        
            // If in vehicle, collision bounds are effectively the vehicle's bounds
            // This might need to be handled by the Vehicle class or GameManager
            return {}; // Return empty rect, or vehicle's bounds
    }
    sf::Vector2f pos = sprite.getPosition();
    sf::Vector2f size(frameWidth * sprite.getScale().x, frameHeight * sprite.getScale().y);

    return {
        pos.x - size.x / 2.f + offset.x,
        pos.y - size.y / 2.f + offset.y,
        size.x,
        size.y
    };
}

sf::Vector2f Player::getCenter() const {
    sf::Vector2f pos = sprite.getPosition();
    return { pos.x - 2.f, pos.y + 4.f };
}

float Player::getCollisionRadius() const {
    // Set to match visual size — adjust as needed
    return 6.f; // in pixels
}


void Player::onCollision(GameObject& other) {
    other.collideWithPlayer(*this); // ← הפוך: אתה Player, שולח את עצמך
}

void Player::collideWithPresent(Present& present) {
    /*if (!present.isCollected()) {
        present.applyEffect(*this);
        present.collect();
    }*/ 
   // std::cout << "collide with present" << std::endl;
}


Inventory& Player::getInventory()
{
    return inventory; 
}

const Inventory& Player::getInventory() const
{
    return inventory;
}

void Player::heal(int amount) {
    if (m_health < MaxHealth) {
        m_health += amount;
        if (m_health > MaxHealth)
            m_health = MaxHealth;

    }
    else {
        
        inventory.addItem("Health", ResourceManager::getInstance().getTexture("Health"));
     
    }
}
void Player::increaseSpeed() {
    std::cout << speedBoostTimer << std::endl;
        if (speedBoostTimer <= 0.f) {
            speed += 30.f;
        speedBoostTimer = 15.f;
    }
    else {
        inventory.addItem("Speed", ResourceManager::getInstance().getTexture("Speed"));
    }
}
void Player::AddAmmo() {
    
}

bool Player::tryBuyAmmo(const std::string& weaponName, int amountToAdd, int price) {
    
    auto it = WeaponsAmmo.find(weaponName);
    if (it == WeaponsAmmo.end()) {
        std::cout << "Weapon not found: " << weaponName << "\n";
        return false;
    }

    AmmoSetting& ammo = it->second;

    if (ammo.Ammo >= ammo.MaxAmmo) {
        std::cout << "Ammo already full for: " << weaponName << "\n";
        return false;
    }

    if (m_money < price) {
        std::cout << "Not enough money for: " << weaponName << " ammo\n";
        return false;
    }

    
    int amountCanAdd = std::min(amountToAdd, ammo.MaxAmmo - ammo.Ammo);

    ammo.Ammo += amountCanAdd;
    m_money -= price;

    std::cout << "Bought " << amountCanAdd << " ammo for " << weaponName << "\n";
    return true;
}

void Player::decreaseMoney(int priceItem)
{
    m_money = std::max(0, m_money - priceItem);
}



void Player::AddWeapon(const std::string name) {
    inventory.addItem(name, ResourceManager::getInstance().getTexture(name));
}



void Player::playAnimation(const std::string& animName, bool loop, bool pingpong) {
    if (!animationManager) return;
    if (currentAnimationName != animName) {
        animationManager->setAnimation(animName, loop, pingpong);
        currentAnimationName = animName;
    }
}

// להציג פריים ספציפי בספרייט שיט
void Player::setSpecificFrame(int row, int col) {
    sf::IntRect rect(col * frameWidth, row * frameHeight, frameWidth, frameHeight);
    sprite.setTextureRect(rect);
}


void Player::enterVehicle(Vehicle* vehicle) {
   
        m_currentVehicle = vehicle;
    // Player sprite could be hidden here, or its position updated to match vehicle
    // For now, Player::draw() handles not drawing the player.
    // Player::update() will sync position.
}

void Player::exitVehicle() {
    if (m_currentVehicle) {
        // Position the player slightly beside the vehicle when exiting
        // This is a simple offset; might need adjustment based on vehicle size/orientation
        sf::Vector2f vehiclePos = m_currentVehicle->getPosition();
        // TODO: Get vehicle's current orientation to place player appropriately (e.g., beside a door)
        // For now, simple offset
        setPosition(sf::Vector2f(vehiclePos.x + 10.f, vehiclePos.y));
        m_currentVehicle = nullptr;
    }
}

Vehicle* Player::getCurrentVehicle() const {
    return m_currentVehicle;
}

bool Player::isInVehicle() const {
    return m_currentVehicle != nullptr;
}

void Player::setCurrentWeapon(const std::string& name, int maxAmmo) {
    m_currentWeaponName = name;
    m_maxWeaponAmmo = maxAmmo;

    // עדכון כמות תחמושת קיימת אם הנשק כבר במפה שלך
    auto it = WeaponsAmmo.find(name);
    if (it != WeaponsAmmo.end()) {
        it->second.MaxAmmo = maxAmmo;
    }
    else {
        // אם הנשק לא קיים עדיין, הוסף אותו
        WeaponsAmmo[name] = AmmoSetting{ 0, maxAmmo };
    }

    std::cout << "Switched weapon to: " << name << " with max ammo: " << maxAmmo << std::endl;
}

int Player::getMoney() const {
    return m_money;
}

int Player::getHealth() const {
    return m_health;
}

int Player::getArmor() const {
    return m_armor;
}

std::string Player::getCurrentWeaponName() const {
    return m_currentWeaponName;
}

int Player::getMaxAmmo() const {
    return m_maxWeaponAmmo;
}

int Player::getWantedLevel() const {
    return m_wantedLevel;
}

void Player::setWantedLevel(int level) {
    m_wantedLevel = level;
}
void Player::collideWithPlayer(Player& /*player*/) {
    // שחקן לא אמור להתנגש בעצמו – לכן אולי לא נדרש טיפול.
    // אפשר להשאיר ריק או להוסיף לוגיקת PVP בעתיד.
}
