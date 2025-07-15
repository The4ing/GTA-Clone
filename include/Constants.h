#pragma once

//gameMnanger
static constexpr float GAME_TIME_SCALE = 60.0f;
static constexpr float MISSION_NEXT_TASK_DELAY = 5.f;

//for the path finding
static constexpr float PATHFINDING_GRID_SIZE = 32.0f;

//size of window
const unsigned int WINDOW_WIDTH = 1920;
const unsigned int WINDOW_HEIGHT = 1080;
const unsigned int MAP_WIDTH = 4640;
const unsigned int MAP_HEIGHT = 4672;


//police + gameManager
const int POLICE_CAR_SPAWN_CHANCE = 15; // 15% chance
const float SPAWN_COOLDOWN_SECONDS = 2.0f;
const sf::FloatRect MAP_BOUNDS(0.f, 0.f, 4640.f, 4670.f);
const float BATON_MELEE_RATE = 1.0f;
const float BATON_MELEE_RANGE = 40.f;
const int BATON_DAMAGE = 10;
const float REPATH_COOLDOWN = 1.0f;
const float PLAYER_MOVE_THRESHOLD_FOR_REPATH_SQ = (PATHFINDING_GRID_SIZE * 2.0f) * (PATHFINDING_GRID_SIZE * 2.0f);
const float TARGET_REACHED_DISTANCE = PATHFINDING_GRID_SIZE;
const float RUN_OVER_DISTANCE = 30.f;

//pistol +bullets
const float PISTOL_FIRE_RATE = 1.5f;
const float PISTOL_SHOOTING_RANGE = 200.f;
const float PISTOL_LINE_OF_SIGHT_RANGE = 250.f;

//HELICOPTER
const float FIRE_RATE = 2.0f;
const float FIRE_RATE_VARIATION = 1.0f;
const float SHOOTING_RANGE = 300.f;
const float IDEAL_DISTANCE_FROM_TARGET = 250.f;

//TANK
const float STOP_DISTANCE = 300.f;
const float CANNON_FIRE_RATE = 5.0f;
const float CANNON_RANGE = 400.f;

// Wanted Level ReductionAdd commentMore actions
const float TIME_TO_START_WANTED_REDUCTION = 30.0f; // Seconds player must be unseen before reduction starts
const float WANTED_REDUCTION_COOLDOWN_SECONDS = 10.0f; // Seconds before another star can be lost

//checking the collision of player for radius
#define M_PI 3.14159265358979323846

//stats for player
const float BasicSpeed = 50.f;
const int MaxHealth = 100;
const int MaxMoney = 100;
const int PlayerMoney = 20;
const int MaxArmor = 100;


//stats for the weapons
struct AmmoSetting {
    int Ammo = 0;
    int MaxAmmo = 0;

};

//range from the store with the player
const float STORE_INTERACT_RADIUS = 60.f;


//for HUD 
const sf::Color GTA_MONEY_GREEN = sf::Color(0, 150, 0);
const sf::Color GTA_HEALTH_PINK = sf::Color(210, 45, 95);
const sf::Color GTA_ARMOR_BLUE = sf::Color(50, 90, 200);
const sf::Color GTA_WEAPON_ORANGE = sf::Color(220, 140, 20);
const sf::Color GTA_AMMO_WHITE = sf::Color(220, 220, 220);
const sf::Color GTA_TIME_WHITE = sf::Color(220, 220, 220);
const sf::Color GTA_STAR_YELLOW = sf::Color(255, 180, 0);
const sf::Color GTA_SHADOW_BLACK = sf::Color(0, 0, 0, 180);

//ALSO HUD
const unsigned int MONEY_CHAR_SIZE = 68;
const unsigned int DEFAULT_CHAR_SIZE = 66;
const unsigned int AMMO_CHAR_SIZE = 72;
const unsigned int WEAPON_CHAR_SIZE = 62;
const unsigned int TIME_CHAR_SIZE = 88;
static const int MAX_STARS = 6;
const sf::Vector2f SHADOW_OFFSET = sf::Vector2f(2.f, 2.f);

//money
static constexpr float MAX_LIFETIME = 5.f;


//for pedestrian
const float animationSpeed = 0.12f;
const float directionChangeInterval = 2.0f;
const float backupDistance = 30.f;
const float deathDuration = 3.f;
static constexpr int frameWidth = 64;
static constexpr int frameHeight = 64;
static constexpr int framesPerRow = 3;
static constexpr int numCharacters = 7;
