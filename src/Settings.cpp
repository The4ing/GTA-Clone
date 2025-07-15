#include "Settings.h"
#include "ResourceManager.h"
#include "SoundManager.h"
#include "Help.h"
#include "StoryScreen.h"
#include <algorithm>

Settings::Settings(sf::RenderWindow& win, sf::Sound& menuSound)
    : window(win), menuMusic(menuSound), soundEnabled(true),
    volumeLevel(SoundManager::getInstance().getVolume() / 100.f), // הגדרת ווליום התחלתי לפי המצב הנוכחי
    brightnessLevel(1.0f), selectedOption(-1)
{
    font = ResourceManager::getInstance().getFont("main");

    // רקע
    background.setTexture(ResourceManager::getInstance().getTexture("background_set"));
    sf::Vector2u textureSize = background.getTexture()->getSize();
    sf::Vector2u windowSize = window.getSize();
    background.setScale(
        static_cast<float>(windowSize.x) / textureSize.x,
        static_cast<float>(windowSize.y) / textureSize.y
    );
    background.setPosition(0.f, 0.f);

    // שכבת כהות לפי הבהירות
    brightnessOverlay.setSize(sf::Vector2f(windowSize));
    brightnessOverlay.setFillColor(sf::Color(0, 0, 0, 0)); // שקוף בהתחלה

    setupOptions(); // יצירת הטקסטים
    setupBars();    // יצירת הפסים
    updateBars();   // עדכון התצוגה ההתחלתית
}

void Settings::setupOptions() {
    std::vector<std::string> texts = {
        "Story", "Help", "Toggle Music", "Volume +", "Volume -",
        "Brightness +", "Brightness -", "Back"
    };

    for (size_t i = 0; i < texts.size(); ++i) {
        sf::Text text;
        text.setFont(font);
        text.setString(texts[i]);
        text.setCharacterSize(i == 0 ? 42 : 32); // הכותרת קצת יותר גדולה
        text.setFillColor(sf::Color::White);
        text.setPosition(100, 100 + i * 60);
        options.push_back(text);
    }

    musicStatusText.setFont(font);
    musicStatusText.setCharacterSize(24);
    musicStatusText.setFillColor(sf::Color::White);
    musicStatusText.setPosition(350, 100 + 2 * 60); // ליד "Toggle Music"
    musicStatusText.setString(SoundManager::getInstance().isMuted() ? "OFF" : "ON");

}

void Settings::setupBars() {
    // הגדרת פסי עוצמה
    volumeBar.setSize(sf::Vector2f(volumeLevel * 200.f, 20.f));
    volumeBar.setFillColor(sf::Color::Green);
    volumeBar.setPosition(400, 100 + 3 * 60);

    brightnessBar.setSize(sf::Vector2f(brightnessLevel * 200.f, 20.f));
    brightnessBar.setFillColor(sf::Color::Blue);
    brightnessBar.setPosition(400, 100 + 5 * 60);
}

void Settings::updateBars() {
    // עדכון האורך לפי הערכים הנוכחיים
    volumeBar.setSize(sf::Vector2f(volumeLevel * 200.f, 20.f));
    brightnessBar.setSize(sf::Vector2f(brightnessLevel * 200.f, 20.f));

    // עדכון שכבת כהות על המסך לפי בהירות
    brightnessOverlay.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>((1.0f - brightnessLevel) * 200)));
}

void Settings::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        auto mousePos = sf::Mouse::getPosition(window);
        for (size_t i = 0; i < options.size(); ++i) {
            if (options[i].getGlobalBounds().contains((float)mousePos.x, (float)mousePos.y)) {
                selectedOption = static_cast<int>(i);
                switch (selectedOption) {
                case 0: showStory(); break;
                case 1: showHelp(); break;
                case 2: toggleSound(); break;
                case 3: increaseVolume(); break;
                case 4: decreaseVolume(); break;
                case 5: increaseBrightness(); break;
                case 6: decreaseBrightness(); break;
                case 7: shouldClose = true; break;
                }
            }
        }
    }
}

void Settings::update() {
    auto mousePos = sf::Mouse::getPosition(window);
    for (auto& text : options) {
        if (text.getGlobalBounds().contains((float)mousePos.x, (float)mousePos.y)) {
            text.setFillColor(sf::Color::Yellow);
            text.setCharacterSize(40);
        }
        else {
            text.setFillColor(sf::Color::White);
            text.setCharacterSize(32);
        }
    }
}

void Settings::draw() {
    window.draw(background);
    for (auto& text : options)
        window.draw(text);
    window.draw(volumeBar);
    window.draw(brightnessBar);
    window.draw(brightnessOverlay);
    window.draw(musicStatusText);

}

// פעולות



void Settings::toggleSound() {
    SoundManager::getInstance().toggleMute();

    // עדכון טקסט על סמך מצב חדש
    if (SoundManager::getInstance().isMuted()) {
        musicStatusText.setString("OFF");
    }
    else {
        musicStatusText.setString("ON");
    }

    // עדכון עוצמת מוזיקה בפועל
    menuMusic.setVolume(SoundManager::getInstance().isMuted() ? 0.f : volumeLevel * 100.f);
}


void Settings::increaseVolume() {
    volumeLevel = std::min(1.0f, volumeLevel + 0.1f);
    updateBars();
    SoundManager::getInstance().setVolume(volumeLevel * 100.f);
    if (soundEnabled)
        menuMusic.setVolume(volumeLevel * 100.f);
}

void Settings::decreaseVolume() {
    volumeLevel = std::max(0.0f, volumeLevel - 0.1f);
    updateBars();
    SoundManager::getInstance().setVolume(volumeLevel * 100.f);
    if (soundEnabled)
        menuMusic.setVolume(volumeLevel * 100.f);
}

void Settings::increaseBrightness() {
    brightnessLevel = std::min(1.0f, brightnessLevel + 0.1f);
    updateBars();
}

void Settings::decreaseBrightness() {
    brightnessLevel = std::max(0.0f, brightnessLevel - 0.1f);
    updateBars();
}

void Settings::showHelp() {
    Help help(window);
    help.run();
}

void Settings::showStory() {
    StoryScreen story(window);
    story.run();
}

bool Settings::shouldExit() const {
    return shouldClose;
}
