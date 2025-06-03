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
    rm.loadTexture("background_menu",  "resources/GtaLogo.png");
    rm.loadTexture("SF", "resources/SF.png");
    rm.loadTexture("SS", "resources/SS.png");
    rm.loadTexture("ST", "resources/ST.png");
    rm.loadTexture("background_set", "resources/set_bk.png");


    // 4️⃣ טוען תמונת StaticObject לדוגמה (כתל/בניין)
    rm.loadTexture("static_building", "resources/building.png");
    rm.loadTexture("map", "resources/map.png"); 

    rm.loadTexture("car", "resources/car.png");
    // 5️⃣ (אופציונלי) תמונת הולך רגל נוספות, כדור, פיצוץ וכדומהerrres\n";



    
}
