#include "StoryScreen.h"
#include "ResourceManager.h"



StoryScreen::StoryScreen(sf::RenderWindow& win)
    : window(win),  currentIndex(0), done(false) 
{
    font = ResourceManager::getInstance().getFont("main");

    fullStory =
        "In a world shattered by conflict,\n"
        "a lone adventurer sets out to restore balance.\n\n"
        "Explore forgotten lands.\n"
        "Face deadly creatures.\n"
        "Uncover the secrets of the ancient world...\n";

    storyText.setFont(font);
    storyText.setCharacterSize(28);
    storyText.setFillColor(sf::Color::White);
    storyText.setPosition(50, 100);

    overlay.setSize(sf::Vector2f(window.getSize()));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));

    background.setTexture(ResourceManager::getInstance().getTexture("background_set"));

    sf::Vector2u textureSize = background.getTexture()->getSize();
    sf::Vector2u windowSize = window.getSize();

    background.setScale(
        static_cast<float>(windowSize.x) / textureSize.x,
        static_cast<float>(windowSize.y) / textureSize.y
    );

    background.setPosition(0.f, 0.f);

}

void StoryScreen::run() {
    while (window.isOpen() && !done) {
        handleEvents();
        update();
        render();
    }
}

void StoryScreen::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
        if (event.type == sf::Event::KeyPressed || event.type == sf::Event::MouseButtonPressed)
            done = true;
    }
}

void StoryScreen::update() {
    if (currentIndex < fullStory.size() && typingClock.getElapsedTime().asMilliseconds() > 30) {
        displayedText += fullStory[currentIndex++];
        storyText.setString(displayedText);
        typingClock.restart();
    }
}

void StoryScreen::render() {
    window.clear();

    window.draw(background);

    window.draw(overlay);
    window.draw(storyText);

    window.display();
}
