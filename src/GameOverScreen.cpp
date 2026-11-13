#include "GameOverScreen.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/Keyboard.hpp>

GameOverScreen::GameOverScreen() {
#ifdef SPACEGAME_FONT_PATH
    fontLoaded = font.openFromFile(SPACEGAME_FONT_PATH);
#endif
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("src/font.ttf");
    }
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("../src/font.ttf");
    }
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("font.ttf");
    }
}

void GameOverScreen::handleEvent(const sf::Event& event) {
    const auto* keyPressed = event.getIf<sf::Event::KeyPressed>();
    if (!keyPressed) {
        return;
    }

    if (keyPressed->code == sf::Keyboard::Key::Up ||
        keyPressed->code == sf::Keyboard::Key::W ||
        keyPressed->code == sf::Keyboard::Key::Down ||
        keyPressed->code == sf::Keyboard::Key::S) {
        selectedOption = 1 - selectedOption;
    } else if (keyPressed->code == sf::Keyboard::Key::Enter) {
        result = selectedOption == 0 ? Result::PlayAgain : Result::MainMenu;
    }
}

void GameOverScreen::render(sf::RenderWindow& window) {
    window.resetGLStates();
    window.clear(sf::Color(10, 14, 22));

    if (fontLoaded) {
        sf::Text title(font, "GAME OVER", 46);
        title.setLetterSpacing(3.0f);
        title.setFillColor(sf::Color(255, 90, 90));
        const sf::FloatRect titleBounds = title.getLocalBounds();
        title.setOrigin({
            titleBounds.position.x + titleBounds.size.x * 0.5f,
            titleBounds.position.y + titleBounds.size.y * 0.5f
        });
        title.setPosition({640.0f, 260.0f});
        window.draw(title);

        const char* labels[] = {"PLAY AGAIN", "MAIN MENU"};
        for (std::size_t i = 0; i < 2; ++i) {
            sf::Text option(font, labels[i], 27);
            option.setFillColor(i == selectedOption ? sf::Color::White : sf::Color(120, 128, 140));
            const sf::FloatRect bounds = option.getLocalBounds();
            const float arrowSpace = i == selectedOption ? 32.0f : 0.0f;
            option.setPosition({
                (1280.0f - bounds.size.x - arrowSpace) * 0.5f + arrowSpace,
                365.0f + static_cast<float>(i) * 58.0f
            });

            if (i == selectedOption) {
                sf::Text arrow(font, ">", 27);
                arrow.setFillColor(sf::Color(90, 235, 255));
                arrow.setPosition({option.getPosition().x - 32.0f, option.getPosition().y});
                window.draw(arrow);
            }
            window.draw(option);
        }

        sf::Text instructions(font, "UP / DOWN TO SELECT    ENTER TO CONFIRM", 13);
        instructions.setLetterSpacing(1.0f);
        instructions.setFillColor(sf::Color(135, 145, 158));
        const sf::FloatRect bounds = instructions.getLocalBounds();
        instructions.setOrigin({
            bounds.position.x + bounds.size.x * 0.5f,
            bounds.position.y + bounds.size.y * 0.5f
        });
        instructions.setPosition({640.0f, 650.0f});
        window.draw(instructions);
    }

    window.display();
}

void GameOverScreen::reset() {
    selectedOption = 0;
    result = Result::None;
}

GameOverScreen::Result GameOverScreen::getResult() const {
    return result;
}