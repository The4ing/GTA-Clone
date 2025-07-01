#include "ResourceInitializer.h"
#include "ResourceManager.h"

void ResourceInitializer::loadInitialResources() {
    auto& rm = ResourceManager::getInstance();

    // Essential resources for menu and loading screen
    rm.loadFont("main", "resources/Pricedown.otf");
    rm.loadTexture("background_menu", "resources/GtaLogo.png");
    rm.loadTexture("loading_background", "resources/SF.png"); // Placeholder for loading screen
}

void ResourceInitializer::loadGameResources() {
    auto& rm = ResourceManager::getInstance();

    // Player textures
    rm.loadTexture("player", "resources/player.png");

    // NPC textures
    rm.loadTexture("pedestrian", "resources/NPC.png");
    rm.loadTexture("police", "resources/PoliceOfficer.png");

    // Other UI/background elements previously in loadAll
    rm.loadTexture("SF", "resources/SF.png"); // Retained if used elsewhere, otherwise can be removed if only for placeholder
    rm.loadTexture("SS", "resources/SS.png");
    rm.loadTexture("ST", "resources/ST.png");
    rm.loadTexture("background_set", "resources/set_bk.png");

    // Weapon Textures
    rm.loadTexture("Pistol", "resources/pistol.png");
    rm.loadTexture("Rifle", "resources/Rifle.png");
    rm.loadTexture("Minigun", "resources/Minigun.png");
    rm.loadTexture("Bazooka", "resources/Bazooka.png");
    rm.loadTexture("Knife", "resources/Knife.png");
    rm.loadTexture("Grenade", "resources/Grenade.png");
    rm.loadTexture("Fists", "resources/fist.png");

    // Item/Present Textures
    rm.loadTexture("Health", "resources/health.png");
    rm.loadTexture("Speed", "resources/speed.png");
    rm.loadTexture("Ammo", "resources/ammo.png");

    // Object Textures
    rm.loadTexture("static_building", "resources/building.png");
    rm.loadTexture("map", "resources/map.png");
 //   rm.loadTexture("bullet", "resources/bullet.png");
    rm.loadTexture("car", "resources/car.png");

    // Sound Buffers
    //rm.loadSoundBuffer("startup", "resources/sounds/startup.wav"); // Example, if you have one
    rm.loadSoundBuffer("gunshot", "resources/sounds/gunshot.wav");
    rm.loadSoundBuffer("rifleShot", "resources/sounds/rifleShot.wav");
    rm.loadSoundBuffer("minigunShot", "resources/sounds/minigunShot.wav");
    rm.loadSoundBuffer("RPGshot", "resources/sounds/RPGshot.wav");
    rm.loadSoundBuffer("KnifeAttack", "resources/sounds/KnifeAttack.wav");
    rm.loadSoundBuffer("ThrowGrenade", "resources/sounds/ThrowGrenade.wav");
    // rm.loadSoundBuffer("explosion", "resources/sounds/explosion.wav");

    //inventory 
    rm.loadTexture("StoreAmmo", "resources/StoreAmmo.png");
    rm.loadTexture("Store", "resources/Store.png");

    //bullets:
    rm.loadTexture("bullet_default", "resources/bullet.png");
    rm.loadTexture("bullet_rifle", "resources/bullet.png");
    rm.loadTexture("bullet_tank", "resources/bullet.png");
    rm.loadTexture("PoliceCar", "resources/policeCar.png");
    rm.loadTexture("policeHelicopter", "resources/policeHelicopter.png");
    rm.loadTexture("policeTank", "resources/policeTank.png");
    rm.loadTexture("tankBody", "resources/tankBody.png");
}
