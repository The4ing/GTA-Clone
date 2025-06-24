#include "AnimationManager.h"
#include <iostream>

AnimationManager::AnimationManager(sf::Sprite& spr, int frameW, int frameH, int cols, int rows)
    : sprite(spr), frameWidth(frameW), frameHeight(frameH), sheetCols(cols), sheetRows(rows),
    currentFrame(0), animTimer(0.f), animDelay(0.1f), currentAnim(""), loop(true), pingpong(false), forward(true)
{
}

void AnimationManager::addAnimation(const std::string& name, AnimationRange range) {
    animations[name] = range;
}
void AnimationManager::initAnimations() {
    addAnimation("Idle_NoWeapon", { 0, 0, 0, 9 });
    addAnimation("ThrowGrenade", { 0, 10, 1, 5 });
    addAnimation("KnifeAttack", { 1, 6, 2, 5 });
    addAnimation("BatAttack", { 2, 6, 2, 11 });
    addAnimation("Hurt", { 3, 0, 3, 3 });
    addAnimation("HurtDie", { 3, 4, 5, 4 });
    addAnimation("RifleShoot", { 5, 5, 5, 8 });
    addAnimation("RifleWalk", { 5, 9, 6, 1 });
    addAnimation("RifleWalkShoot", { 6, 2, 6, 5 });
    addAnimation("PistolShoot", { 6, 6, 6, 9 });
    addAnimation("PistolWalk", { 6, 10, 7, 4 });
    addAnimation("PistolWalkShoot", { 7, 5, 8, 0 });
    addAnimation("MinigunShoot", { 8, 1, 8, 6 });
    addAnimation("MinigunWalk", { 8, 7, 8, 11 });
    addAnimation("MinigunWalkShoot", { 9, 0, 9, 8 });
    addAnimation("BazookaShoot", { 9, 9, 10, 4 });
    addAnimation("BazookaWalk", { 10, 5, 10, 9 });
    addAnimation("BazookaWalkShoot", { 10, 10, 11, 8 });
}

void AnimationManager::setAnimation(const std::string& name, bool shouldLoop, bool isPingPong) {
    if (currentAnim != name) {
        currentAnim = name;
        currentFrame = 0;
        animTimer = 0.f;
        loop = shouldLoop;
        pingpong = isPingPong;
        forward = true;
        updateSpriteRect();
    }
}

void AnimationManager::update(float dt) {
    if (currentAnim.empty()) return;

    animTimer += dt;
    if (animTimer >= animDelay) {
        animTimer -= animDelay;
        AnimationRange& ar = animations[currentAnim];

        int totalFrames = countFrames(ar);

        if (pingpong) {
            if (forward)
                currentFrame++;
            else
                currentFrame--;

            if (currentFrame >= totalFrames) {
                currentFrame = totalFrames - 2;
                forward = false;
            }
            else if (currentFrame < 0) {
                currentFrame = 1;
                forward = true;
            }
        }
        else {
            if (loop)
                currentFrame = (currentFrame + 1) % totalFrames;
            else if (currentFrame < totalFrames - 1)
                currentFrame++;
        }

        updateSpriteRect();
    }
}

void AnimationManager::updateSpriteRect() {
    AnimationRange& ar = animations[currentAnim];

    int totalFrames = countFrames(ar);
    int frameIndex = currentFrame % totalFrames;

    int startIndex = ar.startRow * sheetCols + ar.startCol;
    int frameAbsIndex = startIndex + frameIndex;

    int row = frameAbsIndex / sheetCols;
    int col = frameAbsIndex % sheetCols;

    int left = col * frameWidth;
    int top = row * frameHeight;

    sprite.setTextureRect(sf::IntRect(left, top, frameWidth, frameHeight));

    // Debug info
    std::cout << "Animation: " << currentAnim
        << " | Frame: " << currentFrame
        << " | Row: " << row << " | Col: " << col
        << " | Pixel Rect: (" << left << "," << top << "," << frameWidth << "," << frameHeight << ")\n";
}

int AnimationManager::countFrames(const AnimationRange& ar)const {
    int frames = 0;
    if (ar.startRow == ar.endRow) {
        frames = ar.endCol - ar.startCol + 1;
    }
    else {
        frames += sheetCols - ar.startCol;
        frames += (ar.endRow - ar.startRow - 1) * sheetCols;
        frames += ar.endCol + 1;
    }
    return frames;
}

bool AnimationManager::isAnimationFinished() const {
    if (loop) // ?? ???????? ?????? - ?? ??? ?? ?????
        return false;

    if (currentAnim.empty())
        return true; // ??? ??????? ????? => ????? ??????

    AnimationRange ar = animations.at(currentAnim);
    int totalFrames = countFrames(ar);

    // ?? ????? ?????? ?????? ?? ???? - ???????? ?????
    return currentFrame >= totalFrames - 1;
}
