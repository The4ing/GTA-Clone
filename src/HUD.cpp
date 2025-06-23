// HUD.cpp
#include "HUD.h"
#include <iostream>
#include <cstdio>

HUD::HUD() : m_currentWantedLevel(0) {
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

    // ??? ?????? ?? ????
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

    m_weaponIconRects["Fists"] = sf::IntRect(0, 0, 250, 250);
    m_weaponIconRects["Pistol"] = sf::IntRect(250, 0, 250, 250);
    m_weaponIconRects["Shotgun"] = sf::IntRect(500, 0, 250, 250);
    m_weaponIconRects["Combat Shotgun"] = sf::IntRect(192, 0, 250, 250);

    // ?????? ???????? ????? ?????
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
        float scale = m_starIconSize / m_starTexture.getSize().x;
        m_starSprites[i].setScale(scale, scale);
    }

    updateElementPositions(1920.f, 1080.f);

    // ????? ???????? ???????
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

    const float rightMargin = m_padding;  // Padding from the right edge
    float startX = viewWidth - rightMargin;  // Starting X position near right edge
    float startY = m_padding;                 // Starting Y position near top edge

    // First, position the weapon icon at the top right, aligned to the right edge
    {
        // Position weapon icon so its right edge aligns with 'startX', Y at top padding
        m_weaponIcon.setPosition(startX - m_weaponIcon.getGlobalBounds().width, 0);
        // Move startY down by the icon height for the next element (weapon name)
        startY += m_weaponIcon.getGlobalBounds().height;
    }

    // Position the weapon name text below the weapon icon
    {
        sf::FloatRect bounds = m_weaponNameText.getLocalBounds();
        // Set origin to the right edge so the text aligns right
        m_weaponNameText.setOrigin(bounds.left + bounds.width, bounds.top);
        // Position the weapon name near the right, slightly adjusted vertically
        m_weaponNameText.setPosition(startX - 50, startY - 30);
        // Move startY down by the weapon name text height + padding for next element
        startY += m_weaponNameText.getCharacterSize() + 4.f;
    }

    // Now position the rest of the HUD elements (money, health, armor, ammo, time, stars)
    // to the left of the weapon icon and weapon name text

    // Calculate the starting X coordinate for these elements, offset left from weapon icon
    float leftStartX = startX - m_weaponIcon.getGlobalBounds().width - 20.f; // 20 pixels gap

    float currentY = m_padding; // Reset Y to start from top for these elements

    // Money text (right aligned)
    {
        sf::FloatRect bounds = m_moneyText.getLocalBounds();
        m_moneyText.setOrigin(bounds.left + bounds.width, bounds.top);
        m_moneyText.setPosition(leftStartX, currentY);
        currentY += m_moneyText.getCharacterSize() + 8.f; // Add vertical spacing
    }

    // Health text (right aligned)
    {
        sf::FloatRect bounds = m_healthText.getLocalBounds();
        m_healthText.setOrigin(bounds.left + bounds.width, bounds.top);
        m_healthText.setPosition(leftStartX, currentY);
        currentY += m_healthText.getCharacterSize() + 4.f;
        m_heartSprite.setScale(0.2, 0.2);
        m_heartSprite.setPosition(leftStartX-140, currentY - 75);
    }

    // Armor text (right aligned)
    {
        sf::FloatRect bounds = m_armorText.getLocalBounds();
        m_armorText.setOrigin(bounds.left + bounds.width, bounds.top);
        m_armorText.setPosition(leftStartX, currentY);
        currentY += m_armorText.getCharacterSize() + 12.f;
        m_armorSprite.setScale(0.1, 0.1);
        m_armorSprite.setPosition(leftStartX - 140, currentY - 80);
    }

    // Ammo text (right aligned)
    {
        sf::FloatRect bounds = m_ammoText.getLocalBounds();
        m_ammoText.setOrigin(bounds.left + bounds.width, bounds.top);
        m_ammoText.setPosition(leftStartX, currentY);
        currentY += m_ammoText.getCharacterSize() + 4.f;
    }

    // Time text (centered horizontally at the top)
    {
        sf::FloatRect bounds = m_timeText.getLocalBounds();
        m_timeText.setOrigin(bounds.left + bounds.width / 2.f, bounds.top);
        m_timeText.setPosition(viewWidth / 2.f, m_padding);
    }
}





void HUD::update(const PlayerData& playerData, int wantedLevel, const sf::Time& gameTime) {
    m_moneyText.setString("$" + std::to_string(playerData.money));
    m_healthText.setString(std::to_string(playerData.health)); // ? 100
    m_armorText.setString( std::to_string(playerData.armor));  // ? 0 (?????? ???)
    m_weaponNameText.setString(playerData.weaponName);

    if (playerData.currentAmmo < 0) {
        m_ammoText.setString("");
    }
    else {
        m_ammoText.setString(std::to_string(playerData.currentAmmo));
    }

    m_weaponNameText.setString(playerData.weaponName);

    // ????? ?????? ???? ??? ?? ????
    auto it = m_weaponIconRects.find(playerData.weaponName);
    std::cout << "Updating weapon icon: " << playerData.weaponName << std::endl;
    if (it != m_weaponIconRects.end()) {
        std::cout << "Texture rect: " << it->second.left << ", " << it->second.top
            << ", " << it->second.width << ", " << it->second.height << std::endl;
        m_weaponIcon.setTextureRect(it->second);
        // ????? ????? ?? ????
        //float desiredSize = 48.f; // ???? ???? ?? ??????? ????????
        //float scaleX = desiredSize / it->second.width;
        //float scaleY = desiredSize / it->second.height;
        //m_weaponIcon.setScale(scaleX, scaleY);
    }
    else {
        // ??? ?? ???? - ???? ?????? ?? ?????? ?????? ????? ????
        std::cout << "Weapon icon not found!\n";
        m_weaponIcon.setTextureRect(sf::IntRect()); // ???
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

    // ????? ?????? ??????? ?? ????? ???? / ????
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

    std::cout << "drew \n";
}

void HUD::draw(sf::RenderWindow& window) {
    float viewWidth = static_cast<float>(window.getView().getSize().x);
    float viewHeight = static_cast<float>(window.getView().getSize().y);

    // ????? ?? ?????? ????????
    std::cout << "HUD Elements positions:" << std::endl;
    std::cout << "Money: " << m_moneyText.getPosition().x << ", " << m_moneyText.getPosition().y << std::endl;
    std::cout << "Health: " << m_healthText.getPosition().x << ", " << m_healthText.getPosition().y << std::endl;
    std::cout << "Armor: " << m_armorText.getPosition().x << ", " << m_armorText.getPosition().y << std::endl;
    std::cout << "Weapon Name: " << m_weaponNameText.getPosition().x << ", " << m_weaponNameText.getPosition().y << std::endl;
    std::cout << "Ammo: " << m_ammoText.getPosition().x << ", " << m_ammoText.getPosition().y << std::endl;
    std::cout << "Time: " << m_timeText.getPosition().x << ", " << m_timeText.getPosition().y << std::endl;

    if (m_currentWantedLevel > 0) {
        for (int i = 0; i < m_currentWantedLevel; ++i) {
            std::cout << "Star " << i << ": " << m_starSprites[i].getPosition().x << ", " << m_starSprites[i].getPosition().y << std::endl;
        }
    }

    // ??? ???? ?????
    drawTextWithShadow(window, m_moneyText, SHADOW_OFFSET, GTA_SHADOW_BLACK);
    drawTextWithShadow(window, m_healthText, SHADOW_OFFSET, GTA_SHADOW_BLACK);
    drawTextWithShadow(window, m_armorText, SHADOW_OFFSET, GTA_SHADOW_BLACK);
    drawTextWithShadow(window, m_weaponNameText, SHADOW_OFFSET, GTA_SHADOW_BLACK);
    drawTextWithShadow(window, m_ammoText, SHADOW_OFFSET, GTA_SHADOW_BLACK);
    drawTextWithShadow(window, m_timeText, SHADOW_OFFSET, GTA_SHADOW_BLACK);

    if (m_currentWantedLevel > 0) {
        float starWidth = m_starTexture.getSize().x * m_starSprites[0].getScale().x;
        float totalStarsWidth = (m_currentWantedLevel * starWidth) + ((m_currentWantedLevel - 1) * m_starSpacing);
        // x ????? ??? ????, ???? m_padding ?????, ????? ???? ?? ???????
        float startX = viewWidth - m_padding - totalStarsWidth;
        // y ???? ????? ???? ?? ????? ??? (?????? 5 ???????)
        float startY = m_moneyText.getPosition().y + m_moneyText.getCharacterSize() + 5.f;

        for (int i = 0; i < m_currentWantedLevel; ++i) {
            m_starSprites[i].setColor(GTA_STAR_YELLOW);
            m_starSprites[i].setPosition(startX + i * (starWidth + m_starSpacing), startY);
            window.draw(m_starSprites[i]);
        }
    }
    std::cout << "Weapon Icon position at draw: ("
        << m_weaponIcon.getPosition().x << ", "
        << m_weaponIcon.getPosition().y << ")" << std::endl;
    window.draw(m_heartSprite);
    window.draw(m_armorSprite);
    window.draw(m_weaponIcon); // ????? ?????? ????
}

