#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>

struct AnimationRange {
    int startRow;
    int startCol;
    int endRow;
    int endCol;
};

class AnimationManager {
private:
    sf::Sprite& sprite;
    int frameWidth;
    int frameHeight;
    int sheetCols;
    int sheetRows;

    std::unordered_map<std::string, AnimationRange> animations;
    std::string currentAnim;
    int currentFrame;
    float animTimer;
    float animDelay;

    bool loop = true;       // ???? ??? ??????
    bool pingpong = false;  // ?????? ??????? ???? ????
    bool forward = true;    // ??? ????? ?????/????? (pingpong)

public:
    void initAnimations();
    AnimationManager(sf::Sprite& spr, int frameW, int frameH, int cols, int rows);
    bool isAnimationFinished() const;
    void addAnimation(const std::string& name, AnimationRange range);
    // ???? ?-setAnimation ??????? ?? ????? ????
    void setAnimation(const std::string& name, bool shouldLoop = true, bool isPingPong = false);
    void update(float dt);

private:
    void updateSpriteRect();
    int countFrames(const AnimationRange& ar)const;
};
