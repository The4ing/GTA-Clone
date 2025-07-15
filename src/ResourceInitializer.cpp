#include "ResourceInitializer.h"
#include "ResourceManager.h"

void ResourceInitializer::loadInitialResources() {
    auto& rm = ResourceManager::getInstance();

    // Essential resources for menu and loading screen
    rm.loadFont("main", "resources/Pricedown.otf");
    rm.loadTexture("background_menu", "resources/GtaLogo.png");
    rm.loadTexture("loading_background", "resources/SF.png"); // Placeholder for loading screen
     rm.loadTexture("background_set", "resources/set_bk.png");

    // Other UI/background elements previously in loadAll
    rm.loadTexture("SF", "resources/SF.png"); 
    rm.loadTexture("SS", "resources/SS.png");
   rm.loadTexture("ST", "resources/ST.png");
   //for slide show
   rm.loadSoundBuffer("MenuSound", "resources/sounds/MenuSound.ogg");

    //rm.loadSoundBuffer("LoadScreen", "C:/Users/User/OneDrive/OOP/GTA2/resources/sounds/LoadScreen.ogg");
}

void ResourceInitializer::loadGameResources() {
    auto& rm = ResourceManager::getInstance();

    // Player textures
    rm.loadTexture("player", "resources/player.png");

    // NPC textures
    rm.loadTexture("pedestrian", "resources/NPC.png");
    rm.loadTexture("police", "resources/PoliceOfficer.png");

    
   

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
    rm.loadTexture("Package", "resources/package.png");
    // Object Textures
    rm.loadTexture("static_building", "resources/building.png");
    rm.loadTexture("map", "resources/map.png");
 //   rm.loadTexture("bullet", "resources/bullet.png");
    rm.loadTexture("car_sheet", "resources/cars_spritesheet.png");
    rm.loadTexture("explosion", "resources/explosion.png");
    rm.loadTexture("blood_puddle", "resources/blood_puddle.png");


    // Sound Buffers
    //rm.loadSoundBuffer("startup", "resources/sounds/startup.ogg"); // Example, if you have one
    rm.loadSoundBuffer("mission_complete", "resources/sounds/mission_complete.ogg");
    rm.loadSoundBuffer("gunshot", "resources/sounds/gunshot.ogg");
    rm.loadSoundBuffer("rifleShot", "resources/sounds/rifleShot.ogg");
    rm.loadSoundBuffer("minigunShot", "resources/sounds/minigunShot.ogg");
    rm.loadSoundBuffer("RPGshot", "resources/sounds/RPGshot.ogg");
    rm.loadSoundBuffer("helicopterMove", "resources/sounds/helicopterMove.ogg");
    rm.loadSoundBuffer("helicopterShot", "resources/sounds/helicopterShot.ogg");
    rm.loadSoundBuffer("KnifeAttack", "resources/sounds/KnifeAttack.ogg");
    rm.loadSoundBuffer("ThrowGrenade", "resources/sounds/ThrowGrenade.ogg");
    rm.loadSoundBuffer("noAmmo", "resources/sounds/noAmmo.ogg");
    rm.loadSoundBuffer("NPC_hurt", "resources/sounds/NPChurt.ogg");
    rm.loadSoundBuffer("hurt1", "resources/sounds/hurt1.ogg");
    rm.loadSoundBuffer("hurt2", "resources/sounds/hurt2.ogg");
    rm.loadSoundBuffer("hurt3", "resources/sounds/hurt3.ogg");  
    rm.loadSoundBuffer("hurt4", "resources/sounds/hurt4.ogg");
    rm.loadSoundBuffer("Death1", "resources/sounds/Death1.ogg");
    rm.loadSoundBuffer("Death2", "resources/sounds/Death2.ogg");
    rm.loadSoundBuffer("Death3", "resources/sounds/Death3.ogg");
    rm.loadSoundBuffer("Death4", "resources/sounds/Death4.ogg");
    rm.loadSoundBuffer("notWanted", "resources/sounds/noWantedLevel.ogg");
    rm.loadSoundBuffer("wanted", "resources/sounds/wanted.ogg");
    rm.loadSoundBuffer("dispatch", "resources/sounds/dispatch.ogg");
    rm.loadSoundBuffer("wasted", "resources/sounds/wasted.ogg");
    rm.loadSoundBuffer("gameplay", "resources/sounds/gameplay.ogg");


    // rm.loadSoundBuffer("explosion", "resources/sounds/explosion.ogg");

    //inventory 
    rm.loadTexture("StoreAmmo", "resources/StoreAmmo.png");
    rm.loadTexture("Store", "resources/Store.png");
    rm.loadTexture("Money", "resources/Money.png");

    //bullets:
    rm.loadTexture("bullet_default", "resources/bullet.png"); //pistol or default
    rm.loadTexture("bullet_RPG", "resources/bulletRPG.png");
    rm.loadTexture("bullet_rifle", "resources/bullet.png");
    rm.loadTexture("bullet_tank", "resources/bullet.png");
    rm.loadTexture("PoliceCar", "resources/policeCar.png");
    rm.loadTexture("policeHelicopter", "resources/policeHelicopter.png");
    rm.loadTexture("policeTank", "resources/policeTank.png");
    rm.loadTexture("tankBody", "resources/tankBody.png");
}
