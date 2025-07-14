#pragma once

const float STATIC_POLICE_GRID_SIZE = 500.0f; // Size of the grid for placing static police units
constexpr int LOAD_RADIUS = 2;
static constexpr float PATHFINDING_GRID_SIZE = 32.0f;
const unsigned int WINDOW_WIDTH = 1920;
const unsigned int WINDOW_HEIGHT = 1080;
const unsigned int MAP_WIDTH = 4640;
const unsigned int MAP_HEIGHT = 4672;

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

#define M_PI 3.14159265358979323846

const float BasicSpeed = 50.f;
const int MaxHealth = 100;

const int MaxMoney = 100;
const int PlayerMoney = 20;

const int MaxArmor = 100;


enum CurrentWepapon {
    W_Fists = -1,
    W_Pistol,
    W_Rifle,
    W_Minigun,
    W_Bazooka,
    W_Knife,
    W_Grenade,
};

struct AmmoSetting {
    int Ammo = 0;
    int MaxAmmo = 0;

};

const float STORE_INTERACT_RADIUS = 60.f;
