// HUD.cpp
#include "HUD.h"
#include <iostream>
#include <cstdio>

HUD::HUD() : m_currentWantedLevel(0), m_padding(20.f), m_starIconSize(20.f),
m_starSpacing(4.f),m_viewWidth(1920.f), m_viewHeight(1080.f) {
    m_starSprites.resize(MAX_STARS);
}

bool HUD::loadResources(const std::string& fontPath, const std::string& starTexturePath) {
    if (!m_font.loadFromFile(fontPath)) {
        std::cerr << "Error loading font: " << fontPath << std::endl;
        return false;
    }
    if (!m_starTexture.loadFromFile(starTexturePath)) {
        std::cerr << "Error loading star texture: " << starTexturePath << std::endl;
        return false;
    }
    m_starTexture.setSmooth(true);
    if (!m_weaponIconsTexture.loadFromFile("resources/weapons_spritesheet.png")) {
        std::cerr << "Failed to load weapon icons sprite sheet\n";
        return false;
    }
    m_weaponIconsTexture.setSmooth(true);
    m_weaponIcon.setTexture(m_weaponIconsTexture);

    if (!m_heartTexture.loadFromFile("resources/heart.png")) {
        std::cerr << "Failed to load heart texture\n";
        return false;
    }
    m_heartTexture.setSmooth(true);
    m_heartSprite.setTexture(m_heartTexture);

    if (!m_armorTexture.loadFromFile("resources/armor.png")) {
        std::cerr << "Failed to load armor texture\n";
        return false;
    }
    m_armorTexture.setSmooth(true);
    m_armorSprite.setTexture(m_armorTexture);

    m_weaponIconRects["Fists"] = sf::IntRect(0, 0, 185, 185);
    m_weaponIconRects["Pistol"] = sf::IntRect(185, 0, 185, 185);
    m_weaponIconRects["Rifle"] = sf::IntRect(370, 0, 185, 185);
    m_weaponIconRects["Knife"] = sf::IntRect(550, 0, 185, 185);
    m_weaponIconRects["Grenade"] = sf::IntRect(740, 0, 185, 185);
    m_weaponIconRects["Bazooka"] = sf::IntRect(925, 0, 185, 185);
    m_weaponIconRects["Minigun"] = sf::IntRect(1110, 0, 185, 185);

    m_moneyText.setString("$9999999");
    m_healthText.setString("100");
    m_armorText.setString("100");
    m_weaponNameText.setString("Combat Shotgun");
    m_ammoText.setString("100");
    m_timeText.setString("00:00");

    setupTextElement(m_moneyText, MONEY_CHAR_SIZE, GTA_MONEY_GREEN, { 0, 0 }, true);
    setupTextElement(m_healthText, DEFAULT_CHAR_SIZE, GTA_HEALTH_PINK, { 0, 0 });
    setupTextElement(m_armorText, DEFAULT_CHAR_SIZE, GTA_ARMOR_BLUE, { 0, 0 });
    setupTextElement(m_weaponNameText, WEAPON_CHAR_SIZE, GTA_WEAPON_ORANGE, { 0, 0 }, true);
    setupTextElement(m_ammoText, AMMO_CHAR_SIZE, GTA_AMMO_WHITE, { 0, 0 }, true);
    setupTextElement(m_timeText, TIME_CHAR_SIZE, GTA_TIME_WHITE, { 0, 0 });

    for (int i = 0; i < MAX_STARS; ++i) {
        m_starSprites[i].setTexture(m_starTexture);
       // float scale = m_starIconSize / m_starTexture.getSize().x;
        m_starSprites[i].setScale(0.15, 0.15);

    }

    updateElementPositions(1920.f, 1080.f);

    m_moneyText.setString("$0");
    m_healthText.setString("100");
    m_armorText.setString("0");
    m_weaponNameText.setString("Fists");
    m_ammoText.setString("");

    return true;
}

void HUD::setupTextElement(sf::Text& text, unsigned int charSize, sf::Color color, sf::Vector2f position, bool rightAlign) {
    text.setFont(m_font);
    text.setCharacterSize(charSize);
    text.setFillColor(color);
    if (rightAlign) {
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.left + bounds.width, bounds.top);
    }
    else {
        text.setOrigin(0.f, 0.f);
    }
    text.setPosition(position);
}


void HUD::updateElementPositions(float viewWidth, float viewHeight) {
    m_viewWidth = viewWidth;
    m_viewHeight = viewHeight;

    const float rightMargin = m_padding;
    float startX = viewWidth - rightMargin;
    float startY = m_padding;

    // Weapon Icon
    {
        sf::Vector2f pos(startX - m_weaponIcon.getGlobalBounds().width, 0);
        m_weaponIcon.setPosition(pos);
        startY += m_weaponIcon.getGlobalBounds().height;
    }

    // Weapon Name
    {
        sf::FloatRect bounds = m_weaponNameText.getLocalBounds();
        m_weaponNameText.setOrigin(bounds.left + bounds.width, bounds.top);
        sf::Vector2f pos(startX - 50, startY - 30);
        m_weaponNameText.setPosition(pos);
        startY += m_weaponNameText.getCharacterSize() + 4.f;
    }

    float leftStartX = startX - m_weaponIcon.getGlobalBounds().width - 20.f;
    float currentY = m_padding;

    // Money
    {
        sf::FloatRect bounds = m_moneyText.getLocalBounds();
        m_moneyText.setOrigin(bounds.left + bounds.width, bounds.top);
        sf::Vector2f pos(leftStartX, currentY);
        m_moneyText.setPosition(pos);
        currentY += m_moneyText.getCharacterSize() + 8.f;
    }

    // Health
    {
        sf::FloatRect bounds = m_healthText.getLocalBounds();
        m_healthText.setOrigin(bounds.left + bounds.width, bounds.top);
        sf::Vector2f pos(leftStartX, currentY);
        m_healthText.setPosition(pos);

        m_heartSprite.setScale(0.2, 0.2);
        sf::Vector2f heartPos(leftStartX - 140, currentY - 5);
        m_heartSprite.setPosition(heartPos);

        currentY += m_healthText.getCharacterSize() + 4.f;
    }

    // Armor
    {
        sf::FloatRect bounds = m_armorText.getLocalBounds();
        m_armorText.setOrigin(bounds.left + bounds.width, bounds.top);
        sf::Vector2f pos(leftStartX, currentY);
        m_armorText.setPosition(pos);

        m_armorSprite.setScale(0.1, 0.1);
        sf::Vector2f armorPos(leftStartX - 140, currentY - 5);
        m_armorSprite.setPosition(armorPos);

        currentY += m_armorText.getCharacterSize() + 12.f;
    }

    // Ammo
    {
        sf::FloatRect bounds = m_ammoText.getLocalBounds();
        m_ammoText.setOrigin(bounds.left + bounds.width, bounds.top);
        sf::Vector2f pos(leftStartX, currentY);
        m_ammoText.setPosition(pos);
        currentY += m_ammoText.getCharacterSize() + 4.f;
    }

    // Time
    {
        sf::FloatRect bounds = m_timeText.getLocalBounds();
        m_timeText.setOrigin(bounds.left + bounds.width / 2.f, bounds.top);
        sf::Vector2f pos(viewWidth / 2.f, m_padding);
        m_timeText.setPosition(pos);
    }
    float starWidth = m_starTexture.getSize().x * m_starSprites[0].getScale().x;


    // Wanted Stars Positioning
    if (m_currentWantedLevel > 0) {
        float starWidth = m_starTexture.getSize().x * m_starSprites[0].getScale().x;
        float totalStarsWidth = (m_currentWantedLevel * starWidth) + ((m_currentWantedLevel - 1) * m_starSpacing);
        float startX = m_viewWidth - m_padding - totalStarsWidth;
        float startY = m_moneyText.getPosition().y + m_moneyText.getCharacterSize() + 130.f;

        for (int i = 0; i < m_currentWantedLevel; ++i) {
            m_starSprites[i].setColor(GTA_STAR_YELLOW);
            m_starSprites[i].setPosition(startX + i * (starWidth + m_starSpacing), startY);
        }
    }


}





void HUD::update(const PlayerData& playerData, int wantedLevel, const sf::Time& gameTime) {
    m_moneyText.setString("$" + std::to_string(playerData.money));
    m_healthText.setString(std::to_string(playerData.health)); // ? 100
    m_armorText.setString(std::to_string(playerData.armor));
    m_weaponNameText.setString(playerData.weaponName);

    if (playerData.maxAmmo > 0) {
        m_ammoText.setString(std::to_string(playerData.currentAmmo) + "/" + std::to_string(playerData.maxAmmo));
    }
    else {
        m_ammoText.setString("");
    }
   
    auto it = m_weaponIconRects.find(playerData.weaponName);
    if (it != m_weaponIconRects.end()) {
        m_weaponIcon.setTextureRect(it->second);
    }
    else {
        std::cout << "Weapon icon not found!\n";
        m_weaponIcon.setTextureRect(sf::IntRect()); 
    }

    int totalSeconds = static_cast<int>(gameTime.asSeconds());
    int gameHour = (totalSeconds / 3600) % 24;
    int gameMinute = (totalSeconds / 60) % 60;

    char timeString[6];
    snprintf(timeString, sizeof(timeString), "%02d:%02d", gameHour, gameMinute);
    m_timeText.setString(timeString);

    m_currentWantedLevel = wantedLevel;
    if (m_currentWantedLevel < 0) m_currentWantedLevel = 0;
    if (m_currentWantedLevel > MAX_STARS) m_currentWantedLevel = MAX_STARS;
    {
        sf::FloatRect bounds = m_moneyText.getLocalBounds();
        m_moneyText.setOrigin(bounds.left + bounds.width, bounds.top);
    }
    {
        sf::FloatRect bounds = m_weaponNameText.getLocalBounds();
        m_weaponNameText.setOrigin(bounds.left + bounds.width, bounds.top);
    }
    {
        sf::FloatRect bounds = m_ammoText.getLocalBounds();
        m_ammoText.setOrigin(bounds.left + bounds.width, bounds.top);
    }
    {
        sf::FloatRect bounds = m_timeText.getLocalBounds();
        m_timeText.setOrigin(bounds.left + bounds.width / 2.f, bounds.top);
    }
    updateElementPositions(m_viewWidth, m_viewHeight);
}

void HUD::drawTextWithShadow(sf::RenderWindow& window, sf::Text& text, const sf::Vector2f& offset, const sf::Color& shadowColor) {
    sf::Color originalColor = text.getFillColor();
    text.setFillColor(shadowColor);
    text.move(offset);
    window.draw(text);
    text.move(-offset);
    text.setFillColor(originalColor);
    window.draw(text);
}

void HUD::draw(sf::RenderWindow& window) {
    float viewWidth = static_cast<float>(window.getView().getSize().x);
    float viewHeight = static_cast<float>(window.getView().getSize().y);
    if (m_currentWantedLevel > 0) {
        for (int i = 0; i < m_currentWantedLevel; ++i) {
        }
    }
    if (m_currentWantedLevel > 0) {
        for (int i = 0; i < m_currentWantedLevel; ++i) {
            window.draw(m_starSprites[i]);
        }
    }
    drawTextWithShadow(window, m_moneyText, SHADOW_OFFSET, GTA_SHADOW_BLACK);
    drawTextWithShadow(window, m_healthText, SHADOW_OFFSET, GTA_SHADOW_BLACK);
    drawTextWithShadow(window, m_armorText, SHADOW_OFFSET, GTA_SHADOW_BLACK);
    drawTextWithShadow(window, m_weaponNameText, SHADOW_OFFSET, GTA_SHADOW_BLACK);
    drawTextWithShadow(window, m_ammoText, SHADOW_OFFSET, GTA_SHADOW_BLACK);
    drawTextWithShadow(window, m_timeText, SHADOW_OFFSET, GTA_SHADOW_BLACK);
    window.draw(m_heartSprite);
    window.draw(m_armorSprite);
    window.draw(m_weaponIcon); 
}

