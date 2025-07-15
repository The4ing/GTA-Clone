#pragma once
#include "Present.h"
#include "Player.h"
#include "Constants.h"


class Money : public Present {
public:
    Money(const sf::Texture& texture, const sf::Vector2f& pos);
    std::string getType() const override;

    bool isExpired() const;
    bool getTempMoney() const;

    void applyEffect(class Player& player) override;
    void setTempMoney(bool updt);
private:
    bool tempMoney;
    sf::Clock m_clock;
    


};
