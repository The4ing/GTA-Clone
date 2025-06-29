#pragma once

constexpr int CHUNK_SIZE = 512;
constexpr int LOAD_RADIUS = 2;

const unsigned int WINDOW_WIDTH = 1920;
const unsigned int WINDOW_HEIGHT = 1080;
const unsigned int MAP_WIDTH = 4640;
const unsigned int MAP_HEIGHT = 4672;
const sf::FloatRect MAP_BOUNDS(0.f, 0.f, 4640.f, 4670.f);

#define M_PI 3.14159265358979323846

const float BasicSpeed = 250.f;
const int MaxHealth = 100;

const int MaxMoney = 100;
const int PlayerMoney = 15;

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
