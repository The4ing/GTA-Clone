#include "Store.h"
#include "AmmoRifle.h"
#include <iostream>

Store::Store(const sf::Vector2f& pos) : position(pos), isOpen(false), PlayerClose(false)
{
    font = ResourceManager::getInstance().getFont("main");

    icon.setTexture(ResourceManager::getInstance().getTexture("Store"));
    icon.setScale(0.05f, 0.05f);     
    sf::FloatRect bounds = icon.getLocalBounds();
    icon.setOrigin(bounds.width / 2.f, bounds.height / 2.f);// 🟢 הקטנת גודל האייקון
    icon.setPosition(position);                         // 🟢 מיקום האייקון נקבע לפי position

    Inv.setTexture(ResourceManager::getInstance().getTexture("StoreAmmo"));

    
  
}


void Store::open(Player& player) {
    isOpen = true;
    font = ResourceManager::getInstance().getFont("main");

    items = {
        { 
            "Pistol Ammo",10, sf::IntRect(0, 0, 223, 320),
            [](Player& p) { p.tryBuyAmmo("Pistol", 15, 10); }
        },
        {
            "Rifle Ammo", 20, sf::IntRect(223, 0, 223, 320),
            [](Player& p) { p.tryBuyAmmo("Rifle", 50 ,20); }
        },
        {
            "Minigun Ammo", 50, sf::IntRect(223 * 2 +5, 0, 255, 320),
            [](Player& p) { p.tryBuyAmmo("Minigun", 100,50); }
        },
        {
            "Bazooka Ammo", 20, sf::IntRect(223 * 3 +35, 0, 290, 320),
            [](Player& p) { p.tryBuyAmmo("Bazooka", 1,20); }
        },
        {
            "Armor", 20, sf::IntRect(0, 425, 225, 280),
            [](Player& p) { p.setArmor(100),p.decreaseMoney(20); }
        },
          
        {
            
            "Grenade Ammo", 5,sf::IntRect(223 + 10, 425, 215, 280),
            [](Player& p) { p.tryBuyAmmo("Grenade", 1,20); }
        },

        {
            "Knife Ammo", 5, sf::IntRect(223 * 2 + 10, 425, 200, 280),
            [](Player& p) { p.tryBuyAmmo("Knife", 1,5); }
        },
        {
            "Speed", 5, sf::IntRect(223 * 3 , 425, 200, 280),
            [](Player& p) { p.increaseSpeed(true),p.decreaseMoney(5); }
        },

        {
            "Health", 5, sf::IntRect(223 * 4 +10, 425, 200, 280),
            [](Player& p) { p.heal(100), p.decreaseMoney(5) ; }
        },

    };
}


void Store::update(float dt)
{
}


void Store::drawUI(sf::RenderTarget& target) {
    if (!isOpen) {
        target.draw(icon);
        return;
    }

    sf::RectangleShape blurOverlay(sf::Vector2f(target.getSize()));
    blurOverlay.setFillColor(sf::Color(0, 0, 0, 180));
    target.draw(blurOverlay);

    if (Inv.getTexture()) {
        sf::Vector2u texSize = Inv.getTexture()->getSize();
        Inv.setOrigin(texSize.x / 2.f, texSize.y / 2.f);
        Inv.setPosition(target.getSize().x / 2.f, target.getSize().y / 2.f);
        target.draw(Inv);
    }

    // ← המיקום של העכבר במערכת ה־UI (ולא במשחק)
    sf::Vector2i mousePosPixels = sf::Mouse::getPosition(*dynamic_cast<const sf::RenderWindow*>(&target));

    sf::Vector2f mousePos = target.mapPixelToCoords(mousePosPixels); // לוקח בחשבון view, zoom וכו'

    for (const auto& item : items) {
        // ← מיקום האובייקט בתמונה הכוללת
        sf::Vector2f itemPos = Inv.getPosition() + sf::Vector2f(item.area.left - Inv.getOrigin().x, item.area.top - Inv.getOrigin().y);

        sf::FloatRect itemBounds(itemPos, sf::Vector2f(item.area.width, item.area.height));
        bool isHovered = itemBounds.contains(mousePos);

        sf::RectangleShape border(sf::Vector2f(item.area.width, item.area.height));
        border.setPosition(itemPos);

        if (isHovered) {
            border.setFillColor(sf::Color(255, 255, 255, 50)); // שקוף לבן — הדגשה
            border.setOutlineColor(sf::Color::Cyan);
            border.setOutlineThickness(3.f);
        }
        else {
            border.setFillColor(sf::Color::Transparent);
            border.setOutlineColor(sf::Color::Yellow);
            border.setOutlineThickness(2.f);
        }

        target.draw(border);

        sf::Text priceText("$" + std::to_string(item.price), font, 16);
        priceText.setFillColor(sf::Color::White);
        priceText.setPosition(itemPos.x, itemPos.y + item.area.height + 5);
        target.draw(priceText);
    }

    sf::Text hint("ESC to close", font, 20);
    hint.setFillColor(sf::Color::White);
    hint.setPosition(target.getSize().x / 2.f - 80.f, target.getSize().y - 50.f);
    target.draw(hint);
}




void Store::handleInput(Player& player, const sf::RenderWindow& window) {
    if (!isOpen) return;

    // ← אם הלחצן לחוץ
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        if (!mouseHeld) {  // רק אם זו הלחיצה הראשונה
            mouseHeld = true;

            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);

            for (const auto& item : items) {
                sf::Vector2f itemPos = Inv.getPosition() + sf::Vector2f(item.area.left - Inv.getOrigin().x, item.area.top - Inv.getOrigin().y);
                sf::FloatRect itemBounds(itemPos, sf::Vector2f(item.area.width, item.area.height));

                if (itemBounds.contains(worldPos)) {
                    std::cout << player.getMoney() << " " << item.price << "\n";
                    if (player.getMoney() >= item.price) {
                        item.action(player);
                        std::cout << "Purchased: " << item.name << "\n";
                    }
                    else {
                        std::cout << "Not enough money for: " << item.name << "\n";
                    }
                    break;
                }
            }
        }
    }
    else {
        // ← העכבר שוחרר, אפשר לאפשר לחיצה הבאה
        mouseHeld = false;
    }
}

void Store::setIsOpen(bool opt) {

    isOpen = opt;
}
const bool Store::getPlayerClose() const 
{
    return PlayerClose;
}

void Store::setPlayerClose(bool opt)
{
     PlayerClose = opt;

}

const sf::Vector2f Store::getPosition() const
{
    return position;
}

std::optional<sf::Text> Store::getHintTextIfClose() const {
    if (!PlayerClose) return std::nullopt;

    sf::FloatRect iconBounds = icon.getGlobalBounds();

    // מחשבים את המרכז של האייקון
    float centerX = iconBounds.left + iconBounds.width / 2.f;
    float topY = iconBounds.top;

    // מציבים את הטקסט קצת מעל האייקון
    sf::Vector2f hintPos(centerX, topY - 5.f);

    sf::Text hint("Press E to open store", font, 8);
    hint.setFillColor(sf::Color::White);

    sf::FloatRect textBounds = hint.getLocalBounds();
    hint.setOrigin(textBounds.width / 2.f, textBounds.height); // מרכז אופקי, תחתית אנכית
    hint.setPosition(std::round(hintPos.x), std::round(hintPos.y));

    return hint;
}
//



