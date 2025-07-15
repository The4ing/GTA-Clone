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
   

public:
    AnimationManager(sf::Sprite& spr, int frameW, int frameH, int cols, int rows);

    void initAnimations();
    void addAnimation(const std::string& name, AnimationRange range);
    void setAnimation(const std::string& name, bool shouldLoop = true, bool isPingPong = false, bool forceReset = false);
    void update(float dt);

    bool isAnimationFinished() const;

private:
    void updateSpriteRect();
    int countFrames(const AnimationRange& ar)const;

    sf::Sprite& sprite;
    int frameWidth;
    int frameHeight;
    int sheetCols;
    int sheetRows;
    int currentFrame;
    float animTimer;
    float animDelay;

    std::unordered_map<std::string, AnimationRange> animations;
    std::string currentAnim;


    bool loop;
    bool pingpong;
    bool forward;
};
