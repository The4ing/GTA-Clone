// HUD.h
#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "Constants.h"

struct PlayerData {
    int money = 0;
    int health = 100;
    int armor = 0;
    std::string weaponName = "Fists";
    int maxAmmo = 0;
};

class HUD {
public:
    HUD();
    bool loadResources(const std::string& fontPath, const std::string& starTexturePath);
    void update(const PlayerData& playerData, int wantedLevel, const sf::Time& gameTime);
    void draw(sf::RenderWindow& window);
    void updateElementPositions(float viewWidth, float viewHeight);
    void drawTextWithShadow(sf::RenderWindow& window, sf::Text& text, const sf::Vector2f& offset, const sf::Color& shadowColor);

private:
    void setupTextElement(sf::Text& text, unsigned int charSize, sf::Color color, sf::Vector2f position, bool rightAlign = false);

    sf::Texture m_weaponIconsTexture;   
    sf::Sprite m_weaponIcon;           

    std::map<std::string, sf::IntRect> m_weaponIconRects;  

    sf::Font m_font;
    sf::Texture m_starTexture;

    sf::Text m_moneyText;
    sf::Text m_healthText;
    sf::Text m_armorText;
    sf::Text m_weaponNameText;
    sf::Text m_ammoText;
    sf::Text m_timeText;

    sf::Texture m_heartTexture;
    sf::Sprite m_heartSprite;

    sf::Texture m_armorTexture;
    sf::Sprite m_armorSprite;

    std::vector<sf::Sprite> m_starSprites;

    static const int MAX_STARS = 6;
    int m_currentWantedLevel;


    const sf::Color GTA_MONEY_GREEN = sf::Color(0, 150, 0);
    const sf::Color GTA_HEALTH_PINK = sf::Color(210, 45, 95);
    const sf::Color GTA_ARMOR_BLUE = sf::Color(50, 90, 200);
    const sf::Color GTA_WEAPON_ORANGE = sf::Color(220, 140, 20);
    const sf::Color GTA_AMMO_WHITE = sf::Color(220, 220, 220);
    const sf::Color GTA_TIME_WHITE = sf::Color(220, 220, 220);
    const sf::Color GTA_STAR_YELLOW = sf::Color(255, 180, 0);
    const sf::Color GTA_SHADOW_BLACK = sf::Color(0, 0, 0, 180);


    const unsigned int MONEY_CHAR_SIZE = 68;
    const unsigned int DEFAULT_CHAR_SIZE = 66;
    const unsigned int AMMO_CHAR_SIZE = 72;
    const unsigned int WEAPON_CHAR_SIZE = 62;
    const unsigned int TIME_CHAR_SIZE = 88;

    const sf::Vector2f SHADOW_OFFSET = sf::Vector2f(2.f, 2.f);
    float m_padding = 20.f;
    float m_starIconSize = 20.f;
    float m_starSpacing = 4.f;

    float m_viewWidth = 1920.f;  
    float m_viewHeight = 1080.f;
};
