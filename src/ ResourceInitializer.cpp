#include "ResourceInitializer.h"
#include "ResourceManager.h"

void ResourceInitializer::loadAll() {
    auto& rm = ResourceManager::getInstance();

    // 🅰 פונטים
    rm.loadFont("main", "resources/Pricedown.otf");

    // 2️⃣ טוען תמונת השחקן (player.png) 
   //    וודאו שבמיקום הזה אכן קיים הקובץ
    rm.loadTexture("player", "resources/player.png");

    // 3️⃣ טוען תמונת הולך רגל לדוגמה
    rm.loadTexture("pedestrian", "resources/NPC.png");

    // 4️⃣ טוען תמונת StaticObject לדוגמה (כתל/בניין)
    rm.loadTexture("static_building", "resources/building.png");

    // 5️⃣ (אופציונלי) תמונת הולך רגל נוספות, כדור, פיצוץ וכדומה
    // rm.loadTexture("bullet", "resources/bullet.png");
    // rm.loadTexture("explosion", "resources/explosion.png");

  //  std::cout << "[ResourceInitializer] Loaded fonts and textures\n";



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
