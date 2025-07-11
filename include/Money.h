#pragma once
#include "Present.h"
#include "Player.h"

class Money : public Present {
public:
    Money(const sf::Texture& texture, const sf::Vector2f& pos);
    std::string getType() const override;
    bool isExpired() const;
    void applyEffect(class Player& player) override;
    bool getTempMoney() const ;
    void setTempMoney(bool updt);
private:
    bool tempMoney;
    sf::Clock m_clock;
    static constexpr float MAX_LIFETIME = 5.f; 


};
