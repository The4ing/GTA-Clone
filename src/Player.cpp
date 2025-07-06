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
    m_money(PlayerMoney), m_health(MaxHealth), m_armor(MaxArmor),
    m_currentWeaponName("Fists"), m_maxWeaponAmmo(0),
    m_wantedLevel(0)
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
    m_shooter = std::make_unique<PlayerShooter>(*this, gameManager.getBulletPool());

    WeaponsAmmo = {
    { "Fists",   AmmoSetting{0, 0} },
    { "Pistol",  AmmoSetting{12, 60} },
    { "Rifle",   AmmoSetting{180, 180} },
    { "Minigun", AmmoSetting{1000, 1000} },
    { "Bazooka", AmmoSetting{5, 5} },
    { "Knife",   AmmoSetting{0, 1} },
    { "Grenade", AmmoSetting{3, 10} }
    };
    m_maxWeaponAmmo = WeaponsAmmo[m_currentWeaponName].MaxAmmo;
}

void Player::setPosition(const sf::Vector2f& pos) {
    sprite.setPosition(pos);
}

sf::Vector2f Player::getPosition() const {
    return sprite.getPosition();
}

float Player::getRotation() const {
    return sprite.getRotation();
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

bool Player::consumeAmmo(const std::string& name) {
    auto it = WeaponsAmmo.find(name);
    if (it == WeaponsAmmo.end())
        return false;
    AmmoSetting& ammo = it->second;
    if (ammo.Ammo <= 0)
        return false;
    --ammo.Ammo;
    return true;
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

            bool collision = CollisionUtils::isInsideBlockedPolygon(nextPos, blockedPolygons);

            if (!collision) {
                sprite.move(movement * speed * dt);
            }
        }

        bool shootingAnimPlaying = false;
        if (!currentAnimationName.empty()) {
            bool isShootAnim = currentAnimationName.find("Throw") != std::string::npos ||
                currentAnimationName.find("Shoot") != std::string::npos ||
                currentAnimationName.find("Attack") != std::string::npos;
            // This variable is no longer needed here due to new logic structure
            // shootingAnimPlaying = isShootAnim && !animationManager->isAnimationFinished(); 
        }

        if (isDead) {
            playAnimation("HurtDie", false); // loop = false
        }
        else if (isHit) {
            playAnimation("Hurt", false); // loop = false
        }
        else {
            // Handle shooting first, as it takes precedence and can interrupt other animations
            if (isShooting) {
                bool shootActionTaken = true; // Flag to indicate a shooting animation was triggered
                if (m_currentWeaponName == "Fists") {
                    playAnimation("BatAttack", false, false, true); // loop, pingpong, forceRestart
                }
                else if (m_currentWeaponName == "Pistol") {
                    playAnimation("PistolShoot", false, false, true);
                    m_shooter->shoot(m_currentWeaponName);
                }
                else if (m_currentWeaponName == "Rifle") {
                    playAnimation("RifleShoot", false, false, true);
                    m_shooter->shoot(m_currentWeaponName);
                }
                else if (m_currentWeaponName == "Minigun") {
                    playAnimation("MinigunShoot", false, false, true);
                    m_shooter->shoot(m_currentWeaponName);
                }
                else if (m_currentWeaponName == "Bazooka") {
                    playAnimation("BazookaShoot", false, false, true);
                    m_shooter->shoot(m_currentWeaponName);
                }
                else if (m_currentWeaponName == "Knife") {
                    playAnimation("KnifeAttack", false, false, true);
                    SoundManager::getInstance().playSound("KnifeAttack");
                }
                else if (m_currentWeaponName == "Grenade") {
                    playAnimation("ThrowGrenade", false, false, true);
                    SoundManager::getInstance().playSound("ThrowGrenade");
                }
                else {
                    shootActionTaken = false; // No valid weapon for shooting animation
                }
                // If a shoot action was taken, we typically don't want to immediately switch to idle/walk
                // The animationManager->update(dt) will handle frame progression.
                // The next block will check if an action animation is playing.
            }

            // Check if an action animation (shoot, attack, throw, etc.) is currently playing
            bool isActionAnimationPlaying = false;
            if (!currentAnimationName.empty() && animationManager) { // Ensure animationManager is valid
                bool isAction = currentAnimationName.find("Throw") != std::string::npos ||
                    currentAnimationName.find("Shoot") != std::string::npos ||
                    currentAnimationName.find("Attack") != std::string::npos;
                if (isAction && !animationManager->isAnimationFinished()) {
                    isActionAnimationPlaying = true;
                }
            }

            // If no action animation is playing (or one just finished), then set idle or walking animations
            if (!isActionAnimationPlaying) {
                if (m_currentWeaponName == "Fists") {
                    if (isMoving)
                        playAnimation("Idle_NoWeapon", true); // loop = true
                    else {
                        setSpecificFrame(0, 0);
                        if (!currentAnimationName.empty()) { // If an animation was playing, explicitly stop it
                            playAnimation("", false, false, true);
                        }
                    }
                }
                else if (m_currentWeaponName == "Pistol") {
                    if (isMoving)
                        playAnimation("PistolWalk", true); // loop = true
                    else {
                        setSpecificFrame(6, 10);
                        if (!currentAnimationName.empty()) {
                            playAnimation("", false, false, true);
                        }
                    }
                }
                else if (m_currentWeaponName == "Rifle") {
                    if (isMoving)
                        playAnimation("RifleWalk", true); // loop = true
                    else {
                        setSpecificFrame(5, 5);
                        if (!currentAnimationName.empty()) {
                            playAnimation("", false, false, true);
                        }
                    }
                }
                else if (m_currentWeaponName == "Minigun") {
                    if (isMoving)
                        playAnimation("MinigunWalk", true); // loop = true
                    else {
                        setSpecificFrame(8, 1);
                        if (!currentAnimationName.empty()) {
                            playAnimation("", false, false, true);
                        }
                    }
                }
                else if (m_currentWeaponName == "Bazooka") {
                    if (isMoving)
                        playAnimation("BazookaWalk", true); // loop = true
                    else {
                        setSpecificFrame(9, 9);
                        if (!currentAnimationName.empty()) {
                            playAnimation("", false, false, true);
                        }
                    }
                }
                else if (m_currentWeaponName == "Knife") {
                    // Knife uses Idle_NoWeapon for both moving and idle if not attacking
                    if (isMoving)
                        playAnimation("Idle_NoWeapon", true); // loop = true
                    else
                        playAnimation("Idle_NoWeapon", true); // loop = true, or specific frame for knife idle?
                    // Original was playAnimation("Idle_NoWeapon", true);
                    // For consistency with other idle (non-moving) states that set specific frames,
                    // this might need a specific knife idle frame if desired.
                    // Sticking to original playAnimation call for now.
                }
                else if (m_currentWeaponName == "Grenade") {
                    // Grenade uses Idle_NoWeapon for both moving and idle if not throwing
                    if (isMoving)
                        playAnimation("Idle_NoWeapon", true); // loop = true
                    else
                        playAnimation("Idle_NoWeapon", true); // loop = true
                }
            }
            // If isActionAnimationPlaying is true, we do nothing here, letting the action animation continue.
            // animationManager->update(dt) will advance its frame.
        }
    }

    if (animationManager) { // Ensure animationManager is valid before updating
        animationManager->update(dt);
    }
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
void Player::increaseSpeed(bool ShopItem) {
    std::cout << speedBoostTimer << std::endl;
    if (speedBoostTimer <= 0.f && !ShopItem) {
        speed += 30.f;
        speedBoostTimer = 15.f;
    }
    else {
        inventory.addItem("Speed", ResourceManager::getInstance().getTexture("Speed"));
    }
}
void Player::AddAmmo() {

}


PlayerShooter& Player::getShooter() {
    return *m_shooter;
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
    decreaseMoney(price);


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



void Player::playAnimation(const std::string& animName, bool loop, bool pingpong, bool forceRestart) {
    if (!animationManager) return;
    if (currentAnimationName != animName || forceRestart) {
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

    auto it = WeaponsAmmo.find(name);
    if (it != WeaponsAmmo.end()) {
        m_maxWeaponAmmo = it->second.MaxAmmo;
    }
    else {
        // אם הנשק לא קיים עדיין, הוסף אותו
        WeaponsAmmo[name] = AmmoSetting{ 0, maxAmmo };
        m_maxWeaponAmmo = maxAmmo;
    }

    std::cout << "Switched weapon to: " << name << " with max ammo: " << m_maxWeaponAmmo << std::endl;
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
    auto it = WeaponsAmmo.find(m_currentWeaponName);
    if (it != WeaponsAmmo.end()) {
        return it->second.MaxAmmo;
    }
    return m_maxWeaponAmmo;
}

int Player::getWantedLevel() const {
    return m_wantedLevel;
}

void Player::setArmor(int armor)
{

    m_armor += armor;
    if (m_armor > MaxArmor)
        m_armor = MaxArmor;

}
void Player::setWantedLevel(int level) {
    m_wantedLevel = level;
}
void Player::collideWithPlayer(Player& /*player*/) {
    // שחקן לא אמור להתנגש בעצמו – לכן אולי לא נדרש טיפול.
    // אפשר להשאיר ריק או להוסיף לוגיקת PVP בעתיד.
}
