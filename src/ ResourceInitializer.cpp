#include "ResourceInitializer.h"
#include "ResourceManager.h"

void ResourceInitializer::loadAll() {
    auto& rm = ResourceManager::getInstance();

    // 🅰 פונטים
    rm.loadFont("main", "C:/Users/User/OneDrive/OOP/GTA/resources/Miskan.ttf");



    // 🅱 טקסטורות דמויות
    //rm.loadTexture("player", "assets/player.png");
    //rm.loadTexture("npc", "assets/npc.png");
    //rm.loadTexture("police", "assets/police.png");
    //rm.loadTexture("pedestrian", "assets/pedestrian.png");

    //// 🅲 טקסטורות רכבים
    //rm.loadTexture("car", "assets/vehicles/car.png");
    //rm.loadTexture("truck", "assets/vehicles/truck.png");

    //// 🅳 קליעים ופיצוצים
    //rm.loadTexture("bullet", "assets/bullet.png");
    //rm.loadTexture("explosion", "assets/explosion.png");

    //// 🅴 עצמים נייחים
    //rm.loadTexture("barrel", "assets/props/barrel.png");
    //rm.loadTexture("building", "assets/props/building.png");

    // הוספת משאבים נוספים בהמשך...
}
