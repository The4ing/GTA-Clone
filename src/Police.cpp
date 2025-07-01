#include "Police.h"
#include "ResourceManager.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include "CollisionUtils.h"
#include "GameManager.h" 
#include "Player.h"       // Included for Player type
#include <cmath>        // For std::atan2, std::cos, std::sin, std::hypot
#include <iostream>     // For std::cerr (error logging)

Police::Police(GameManager & gameManager, PoliceWeaponType weaponType) :
    m_gameManager(gameManager),
    m_weaponType(weaponType), // Initialize weapon type
    pathfinder(*gameManager.getPathfindingGrid()),
    state(PoliceState::Idle),
    currentPathIndex(0), repathTimer(0.f), pathFailCooldown(0.f),
    fireCooldownTimer(0.f), meleeCooldownTimer(0.f)
    // animationTimer(0.f), animationSpeed(0.005f), currentFrame(0) // Likely handled by AnimationManager
{
    if (!gameManager.getPathfindingGrid()) {
        std::cerr << "CRITICAL: PathfindingGrid is null in Police constructor!" << std::endl;
    }
    sprite.setTexture(ResourceManager::getInstance().getTexture("police"));
    // Assuming sheetCols, sheetRows, frameWidth, frameHeight are correctly set in header or based on texture
    // If these are member variables, ensure they are initialized before use here.
    // For example, if texture size is needed:
    const sf::Texture* tex = sprite.getTexture();
    if (tex) {
        frameWidth = tex->getSize().x / sheetCols; // sheetCols from Police.h
        frameHeight = tex->getSize().y / sheetRows; // sheetRows from Police.h
    }
    else {
        // Fallback or error if texture not loaded
        frameWidth = 32;
        frameHeight = 32;
        std::cerr << "Police texture not loaded, using default frame size." << std::endl;
    }

    sprite.setTextureRect({ 0, 0, frameWidth, frameHeight });
    sprite.setOrigin(static_cast<float>(frameWidth) / 2.f, static_cast<float>(frameHeight) / 2.f);
    sprite.setScale(0.15f, 0.15f); // Ensure this scale is appropriate for collisionRadius
    // speed = 40.f; // Initialized in header
    pathTargetPosition = sf::Vector2f(-1, -1);

    nextPauseTime = 30.f + static_cast<float>(rand()) / (RAND_MAX / 30.f);


    animationManager = std::make_unique<AnimationManager>(sprite, frameWidth, frameHeight, sheetCols, sheetRows);
    initAnimations(); // Call after animationManager is created

    setRandomWanderDestination(MAP_BOUNDS); // MAP_BOUNDS needs to be accessible
}

void Police::handleShooting(Player& player, float dt) {
    sf::Vector2f playerPosition = player.getPosition();
    sf::Vector2f aimDir = playerPosition - getPosition();
    float aimDirLen = std::hypot(aimDir.x, aimDir.y);
    if (aimDirLen > 0) {
        aimDir /= aimDirLen;
    }

    float angle = std::atan2(aimDir.y, aimDir.x) * 180.f / M_PI;
    sprite.setRotation(angle + 90.f);

    if (fireCooldownTimer <= 0.f) {
        m_gameManager.addBullet(getPosition() + aimDir * 20.f, aimDir);
        fireCooldownTimer = PISTOL_FIRE_RATE;
        // TODO: Ensure "Shoot_Gun" animation exists and is played
        animationManager->setAnimation("Walk_Gun_1", false); // Placeholder, use actual shooting animation
    }
}

void Police::handleMeleeAttack(Player& player, float dt) {
    sf::Vector2f playerPosition = player.getPosition();
    sf::Vector2f aimDir = playerPosition - getPosition();
    float angle = std::atan2(aimDir.y, aimDir.x) * 180.f / M_PI;
    sprite.setRotation(angle + 90.f); // Face the player

    if (meleeCooldownTimer <= 0.f) {
        // std::cout << "Police " << this << " attacking player with baton!" << std::endl;
        player.takeDamage(BATON_DAMAGE); // Player needs takeDamage method
        meleeCooldownTimer = BATON_MELEE_RATE;
        animationManager->setAnimation("Baton_Attack", false); // Play baton attack animation
    }
}


void Police::update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    setTargetPosition(player.getPosition()); // Update target with player's current position
    repathTimer += dt;

    // Cooldowns
    if (fireCooldownTimer > 0.f) fireCooldownTimer -= dt;
    if (meleeCooldownTimer > 0.f) meleeCooldownTimer -= dt;
    if (pathFailCooldown > 0.f) pathFailCooldown -= dt;

    // Pause logic (existing)
    if (isPaused) {
        pauseTimer -= dt;
        if (pauseTimer <= 0.f) {
            isPaused = false;
            nextPauseTime = 30.f + static_cast<float>(rand()) / (RAND_MAX / 30.f);
        }
        else {
            animationManager->setAnimation("Idle_NoWeapon", true); // Or appropriate idle for weapon
            animationManager->update(dt);
            return;
        }
    }
    else {
        nextPauseTime -= dt;
        if (nextPauseTime <= 0.f) {
            isPaused = true;
            pauseTimer = 3.0f;
            animationManager->setAnimation("Idle_NoWeapon", true);
            animationManager->update(dt);
            return;
        }
    }

    float distToPlayer = std::hypot(targetPos.x - getPosition().x, targetPos.y - getPosition().y);

    // --- State Transition Logic ---
    // If actively attacking, check if should continue or switch to chasing
    if (state == PoliceState::Shooting) {
        if (m_weaponType != PoliceWeaponType::PISTOL || distToPlayer > PISTOL_LINE_OF_SIGHT_RANGE || distToPlayer > PISTOL_SHOOTING_RANGE + 20.f) {
            state = PoliceState::Chasing;
            currentPath.clear(); currentPathIndex = 0; pathTargetPosition = sf::Vector2f(-1, -1);
        }
        else {
            handleShooting(player, dt);
            animationManager->update(dt);
            return; // Skip movement while shooting
        }
    }
    else if (state == PoliceState::MeleeAttacking) {
        if (m_weaponType != PoliceWeaponType::BATON || distToPlayer > BATON_MELEE_RANGE + 10.f) { // Small buffer to stop melee
            state = PoliceState::Chasing;
            currentPath.clear(); currentPathIndex = 0; pathTargetPosition = sf::Vector2f(-1, -1);
        }
        else {
            handleMeleeAttack(player, dt);
            animationManager->update(dt);
            return; // Skip movement while melee attacking
        }
    }

    // Determine new state based on conditions (if not already attacking)
    bool canAttack = false;
    if (m_weaponType == PoliceWeaponType::PISTOL && distToPlayer <= PISTOL_SHOOTING_RANGE && distToPlayer <= PISTOL_LINE_OF_SIGHT_RANGE) {
        if (state != PoliceState::Shooting) {
            state = PoliceState::Shooting;
            currentPath.clear(); currentPathIndex = 0;
            // std::cout << "Police " << this << " entering PISTOL shooting state." << std::endl;
        }
        canAttack = true;
    }
    else if (m_weaponType == PoliceWeaponType::BATON && distToPlayer <= BATON_MELEE_RANGE) {
        if (state != PoliceState::MeleeAttacking) {
            state = PoliceState::MeleeAttacking;
            currentPath.clear(); currentPathIndex = 0;
            // std::cout << "Police " << this << " entering BATON melee state." << std::endl;
        }
        canAttack = true;
    }

    if (!canAttack) { // If cannot attack, decide if chasing or idle
        if (distToPlayer <= detectionRadius) {
            if (state != PoliceState::Chasing) {
                // std::cout << "Police " << this << " entering CHASING state." << std::endl;
                state = PoliceState::Chasing;
                currentPath.clear(); currentPathIndex = 0; pathTargetPosition = sf::Vector2f(-1, -1);
                repathTimer = 1.0f; // Force repath soon
            }
        }
        else if (state == PoliceState::Chasing && distToPlayer > detectionRadius + 30.f) {
            // std::cout << "Police " << this << " leaving CHASING, entering IDLE state." << std::endl;
            state = PoliceState::Idle;
            currentPath.clear(); currentPathIndex = 0;
            setRandomWanderDestination(MAP_BOUNDS);
        }
        else if (state != PoliceState::Chasing && state != PoliceState::BackingUp) { // Ensure not to interrupt backing up by setting to idle
            state = PoliceState::Idle; // Default to idle if not attacking or chasing
        }
    }


    // Backing up logic (existing, ensure it doesn't conflict)
    if (state == PoliceState::BackingUp) {
        float stepSize = speed * dt;
        if (backedUpSoFar < backUpDistance) {
            // ... (existing backing up movement code) ...
            // Ensure sprite rotation is correct for backing up
            // animationManager->setAnimation("BackingUp", true); // Make sure this animation exists
            animationManager->update(dt);
            return;
        }
        else {
            backedUpSoFar = 0.f;
            state = PoliceState::Idle; // Or Chasing if player is still close
            currentPath.clear(); currentPathIndex = 0;
            setRandomWanderDestination(MAP_BOUNDS); // Or repath to player
            animationManager->update(dt);
            return;
        }
    }

    // --- Pathfinding and Movement (only if not attacking or backing up) ---
    if (state == PoliceState::Chasing || state == PoliceState::Idle) {
        const float SIGNIFICANT_PLAYER_MOVE_THRESHOLD_SQ = (PATHFINDING_GRID_SIZE * 3.5f) * (PATHFINDING_GRID_SIZE * 3.5f);
        bool needsNewPath = currentPath.empty() || currentPathIndex >= currentPath.size();
        sf::Vector2f currentPathDest = (state == PoliceState::Chasing) ? targetPos : wanderDestination;

        if (!needsNewPath && state == PoliceState::Chasing && pathTargetPosition.x != -1.f) {
            float dx = targetPos.x - pathTargetPosition.x;
            float dy = targetPos.y - pathTargetPosition.y;
            if ((dx * dx + dy * dy) > SIGNIFICANT_PLAYER_MOVE_THRESHOLD_SQ) {
                needsNewPath = true; // Player moved significantly
            }
        }

        bool repathTimerUp = (state == PoliceState::Chasing) ? (repathTimer > 1.0f) : (repathTimer > 0.5f);

        if ((needsNewPath || (repathTimerUp && (state == PoliceState::Chasing || pathTargetPosition.x == -1.f))) && pathFailCooldown <= 0.f) {
            if (PoliceManager::canRequestPath()) {
                PoliceManager::recordPathfindingCall();
                currentPath = pathfinder.findPath(getPosition(), currentPathDest);
                currentPathIndex = 0;
                repathTimer = 0.f;
                if (!currentPath.empty()) {
                    pathTargetPosition = currentPathDest;
                }
                else {
                    pathFailCooldown = (state == PoliceState::Chasing) ? 3.0f : 1.0f;
                    pathTargetPosition = sf::Vector2f(-1.f, -1.f);
                    if (state == PoliceState::Idle) setRandomWanderDestination(MAP_BOUNDS);
                }
            }
            else {
                repathTimer = 0.f; // Throttled, try again soon
            }
        }

        // Movement Execution
        if (!currentPath.empty() && currentPathIndex < currentPath.size()) {
            sf::Vector2f nextWaypoint = currentPath[currentPathIndex];
            bool hitObstacle = moveToward(nextWaypoint, dt, blockedPolygons);

            if (hitObstacle) {
                state = PoliceState::BackingUp;
                backedUpSoFar = 0.f;
                sf::Vector2f dirToNext = nextWaypoint - getPosition();
                float len = std::hypot(dirToNext.x, dirToNext.y);
                if (len > 0.01f) backUpDirection = -dirToNext / len;
                else backUpDirection = sf::Vector2f(0, 1); // Default backup if on top of point
                // animationManager->setAnimation("BackingUp", true); // Ensure this exists
                animationManager->update(dt);
                return;
            }

            float distanceToWaypoint = std::hypot(nextWaypoint.x - getPosition().x, nextWaypoint.y - getPosition().y);
            if (distanceToWaypoint < PATHFINDING_GRID_SIZE / 2.0f) {
                currentPathIndex++;
                if (currentPathIndex >= currentPath.size()) {
                    currentPath.clear(); currentPathIndex = 0;
                    if (state == PoliceState::Idle) setRandomWanderDestination(MAP_BOUNDS);
                }
            }
        }
    }

    // --- Animation ---
    // Animation selection based on state and weapon
    bool isMoving = !currentPath.empty() && currentPathIndex < currentPath.size() && (state == PoliceState::Chasing || state == PoliceState::Idle);
    if (state == PoliceState::Idle) {
        animationManager->setAnimation(isMoving ? "Walk_NoWeapon" : "Idle_NoWeapon", true);
    }
    else if (state == PoliceState::Chasing) {
        if (m_weaponType == PoliceWeaponType::PISTOL) {
            animationManager->setAnimation(isMoving ? "Walk_Gun_1" : "Idle_Gun_1", true); // Assuming Idle_Gun_1 exists
        }
        else { // BATON or other non-gun equipped for chasing
            animationManager->setAnimation(isMoving ? "Walk_NoWeapon" : "Idle_NoWeapon", true); // Or a specific baton-ready walk/idle
        }
    }
    else if (state == PoliceState::BackingUp) {
        animationManager->setAnimation("Walk_NoWeapon", true); // Or a specific "BackingUp" animation
    }
    // Attacking states (Shooting, MeleeAttacking) handle their animations in their respective handlers or earlier in update.

    animationManager->update(dt);
}


bool Police::moveToward(const sf::Vector2f& target, float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    sf::Vector2f currentPos = getPosition();
    sf::Vector2f dir = target - currentPos;
    float len = std::hypot(dir.x, dir.y); // Use std::hypot
    if (len < 0.01f) return false;

    dir /= len; // Normalize

    float moveDistance = speed * dt;
    // float stepSize = 5.f; // This iterative collision checking might be too slow or complex.
                          // Prefer relying on pathfinder to generate paths around static obstacles.
                          // Dynamic obstacle avoidance (other units) is a more complex topic.
                          // For now, direct move and simple collision check at endpoint.

    sf::Vector2f nextPosCandidate = currentPos + dir * std::min(moveDistance, len);

    // Simplified collision: Check only the final candidate position for this step.
    // More robust would be sweep test or multiple intermediate checks if step is large.
    if (checkCollision(currentPos, nextPosCandidate, blockedPolygons, getCollisionRadius())) {
        // std::cout << "Police: Collision detected at nextPosCandidate" << std::endl;
        return true; // Collision detected
    }

    // No collision, update position
    sprite.setPosition(nextPosCandidate);

    // Update rotation only if not in an attack state that controls rotation
    if (state != PoliceState::Shooting && state != PoliceState::MeleeAttacking) {
        float angle = std::atan2(dir.y, dir.x) * 180.f / M_PI;
        // The rotation offset (-270.f or +90.f) depends on how the sprite is oriented by default.
        // Assuming sprite faces upwards (0 degrees north), then +90 makes it face right for atan2.
        // If sprite faces right by default, then no offset or atan2(dir.y, dir.x) directly.
        // Previous code used angle - 270.f and angle + 90.f. Let's stick to one for movement.
        // angle + 90.f seems to be used in handleShooting/Melee.
        sprite.setRotation(angle + 90.f);
    }

    return false; // No collision
}



void Police::draw(sf::RenderTarget& window) {
    window.draw(sprite);
}

void Police::takeDamage(int amount) {
    health -= amount;
    if (health < 0) health = 0;
}

bool Police::isDead() const {
    return health <= 0;
}

float Police::getCollisionRadius() const {
    // Effective radius after scaling. Original frameWidth * scale / 2
    return (frameWidth * sprite.getScale().x) / 2.0f * 0.8f; // 0.8f is an adjustment factor
}

void Police::setTargetPosition(const sf::Vector2f& pos) {
    if (targetPos != pos) {
        targetPos = pos;
        // Optionally, force a repath if the target moves significantly
        // repathTimer = 1.0f; // Or some other logic
        // currentPath.clear(); // If player moves, old path is invalid
        // currentPathIndex = 0;
    }
}

void Police::setRandomWanderDestination(const sf::FloatRect& mapBounds) {
    sf::Vector2f candidate;
    bool found = false;
    int tries = 0;
    const int MAX_TRIES = 20;

    // Try to find a point that is somewhat distant and within map bounds
    do {
        float angle = static_cast<float>(rand()) / RAND_MAX * 2.f * static_cast<float>(M_PI);
        float radius = 100.f + static_cast<float>(rand()) / RAND_MAX * 400.f; // Wander between 100 and 500 units away
        candidate = getPosition() + sf::Vector2f(std::cos(angle), std::sin(angle)) * radius;

        if (mapBounds.contains(candidate)) {
            // Optional: Could add a check here to see if candidate is inside a blockedPolygon, though pathfinder should handle it.
            // For simplicity, we assume pathfinder will find a way or fail gracefully.
            found = true;
        }
        tries++;
    } while (!found && tries < MAX_TRIES);

    if (found) {
        wanderDestination = candidate;
    }
    else {
        // Fallback: if many tries fail, pick a point towards center or a default safe spot
        wanderDestination = sf::Vector2f(mapBounds.width / 2.f, mapBounds.height / 2.f);
    }
    //std::cout << "Police (Idle): New wander destination set to (" << wanderDestination.x << ", " << wanderDestination.y << ")" << std::endl;
    currentPath.clear();      // Clear old path
    currentPathIndex = 0;     // Reset index
    repathTimer = 0.f;        // Allow immediate repathing for new wander destination
    pathFailCooldown = 0.f;   // Reset path fail cooldown
}

bool Police::checkCollision(const sf::Vector2f& currentPos, const sf::Vector2f& nextPos, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons, float radius) {
    // בדוק קטע בין currentPos ל-nextPos או נקודות ביניים
    const int steps = 5;
    for (int i = 1; i <= steps; ++i) {
        float t = i / float(steps);
        sf::Vector2f interpPos = currentPos + (nextPos - currentPos) * t;

        // אפשר לבדוק אם circle סביב interpPos חופף פוליגון חסום
        for (const auto& poly : blockedPolygons) {
            if (CollisionUtils::pointInPolygon(interpPos, poly)) {
                return true; // התנגשות
            }
        }
    }
    return false;
}
void Police::initAnimations() {
    animationManager->addAnimation("Walk_NoWeapon", { 0, 0, 1, 6 });       // שורה 0 עד 1, עמודה 0 עד 6
    animationManager->addAnimation("Walk_Gun_1", { 1, 7, 3, 3 });       // שורה 1 עד 3, עמודה 7 עד 3 (שים לב: צריך לשים לב לסדר, כנראה {1,7,3,3} משמעותו שורה 1 עד 3, עמודה 7 עד 3 - אם זה לא הגיוני אפשר לתקן)
    animationManager->addAnimation("Walk_Gun_2", { 3, 4, 4, 8 });       // שורה 3 עד 4, עמודה 4 עד 8
    animationManager->addAnimation("Walk_Shield", { 4, 9, 6, 9 });       // שורה 4 עד 6, עמודה 9
    animationManager->addAnimation("Throw_Grenade", { 7, 0, 7, 9 });       // שורה 7, עמודה 0 עד 9
    animationManager->addAnimation("Baton_Attack", { 8, 0, 8, 8 });       // שורה 8, עמודה 0 עד 8
    animationManager->addAnimation("Dying", { 8, 9, 9, 7 });       // שורה 8 עד 9, עמודה 9 עד 7 (גם כאן, אם סדר העמודות הפוך יש לתקן)
}

void Police::setSpecificFrame(int row, int col) {
    sf::IntRect rect(col * frameWidth, row * frameHeight, frameWidth, frameHeight);
    sprite.setTextureRect(rect);
}