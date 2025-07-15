#include "Slideshow.h"
#include "ResourceManager.h"
#include <iostream>


Slideshow::Slideshow(sf::RenderWindow& win, float duration)
    : window(win), durationPerImage(duration) {

}

void Slideshow::run() {
    sf::Sprite sprite;
    sf::RectangleShape overlay;
    overlay.setSize(static_cast<sf::Vector2f>(window.getSize()));
    overlay.setFillColor(sf::Color(0, 0, 0, 255));  // שחור, אטום

   

    for (const auto& key : imageKeys) {
        const sf::Texture& texture = ResourceManager::getInstance().getTexture(key);
        sprite.setTexture(texture);

        sf::Vector2u textureSize = texture.getSize();
        sf::Vector2u windowSize = window.getSize();

        sprite.setScale(
            static_cast<float>(windowSize.x) / textureSize.x,
            static_cast<float>(windowSize.y) / textureSize.y
        );
        sprite.setPosition(0, 0);

        for (int alpha = 255; alpha >= 0; alpha -= 5) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
            }

            overlay.setFillColor(sf::Color(0, 0, 0, alpha));
            window.clear();
            window.draw(sprite);
            window.draw(overlay);
            window.display();
            sf::sleep(sf::milliseconds(10));
        }

        sf::Clock clock;
        while (clock.getElapsedTime().asSeconds() < durationPerImage) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
            }

            window.clear();
            window.draw(sprite);
            window.display();
        }

        for (int alpha = 0; alpha <= 255; alpha += 5) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
            }

            overlay.setFillColor(sf::Color(0, 0, 0, alpha));
            window.clear();
            window.draw(sprite);
            window.draw(overlay);
            window.display();
            sf::sleep(sf::milliseconds(10));
        }
    }

    
}
