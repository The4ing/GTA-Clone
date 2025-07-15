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
    int currentAmmo = 0;
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

    sf::Texture m_starTexture;
    sf::Texture m_weaponIconsTexture;  
    sf::Texture m_heartTexture;
    sf::Texture m_armorTexture;

    sf::Sprite m_weaponIcon;           
    sf::Sprite m_armorSprite;
    sf::Sprite m_heartSprite;
    std::vector<sf::Sprite> m_starSprites;

    std::map<std::string, sf::IntRect> m_weaponIconRects;  

    sf::Font m_font;

    sf::Text m_moneyText;
    sf::Text m_healthText;
    sf::Text m_armorText;
    sf::Text m_weaponNameText;
    sf::Text m_ammoText;
    sf::Text m_timeText;

   
    int m_currentWantedLevel;

    float m_padding ;
    float m_starIconSize ;
    float m_starSpacing;
    float m_viewWidth ;  
    float m_viewHeight ;
};
