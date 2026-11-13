#include "TitleScreen.hpp"
#include "ships/Ship.hpp"

#include <SFML/Window/Keyboard.hpp>

#include <cmath>
#include <string>

namespace {
constexpr unsigned int WINDOW_WIDTH = 1280;
constexpr unsigned int WINDOW_HEIGHT = 720;
constexpr float PI = 3.14159265358979323846f;
}

TitleScreen::TitleScreen() : fighter(), renderer(WINDOW_WIDTH, WINDOW_HEIGHT) {
    fontLoaded = font.openFromFile("src/font.ttf");
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("../src/font.ttf");
    }
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("font.ttf");
    }

    if (fontLoaded) {
        menuText.emplace_back(font, "PLAY", 30);
        menuText.emplace_back(font, "EXIT", 30);

        for (sf::Text& text : menuText) {
            text.setLetterSpacing(1.5f);
        }

        updateMenuText();
    }
}

void TitleScreen::handleEvent(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::Up ||
            keyPressed->code == sf::Keyboard::Key::W) {
            moveSelection(-1);
        } else if (keyPressed->code == sf::Keyboard::Key::Down ||
                   keyPressed->code == sf::Keyboard::Key::S) {
            moveSelection(1);
        } else if (keyPressed->code == sf::Keyboard::Key::Enter) {
            result = selectedOption == static_cast<std::size_t>(MenuOption::Play)
                ? Result::Play
                : Result::Exit;
        } else if (keyPressed->code == sf::Keyboard::Key::Escape) {
            result = Result::Exit;
        }
    }
}

void TitleScreen::moveSelection(int direction) {
    constexpr std::size_t optionCount = 2;

    const int count = static_cast<int>(optionCount);
    int next = static_cast<int>(selectedOption) + direction;

    if (next < 0) {
        next = count - 1;
    } else if (next >= count) {
        next = 0;
    }

    selectedOption = static_cast<std::size_t>(next);
    updateMenuText();
}

void TitleScreen::update(float deltaTime) {
    yaw += 0.35f * deltaTime;
    pitch = std::sin(yaw) * (5.0f * PI / 180.0f);
    roll = std::sin(yaw) * (30.0f * PI / 180.0f);
}

void TitleScreen::updateMenuText() {
    if (!fontLoaded) {
        return;
    }

    for (std::size_t i = 0; i < menuText.size(); ++i) {
        const bool selected = i == selectedOption;

        menuText[i].setFillColor(selected ? sf::Color::White : sf::Color(115, 120, 130));

        menuText[i].setPosition({
            0.0f,
            500.0f + static_cast<float>(i) * 55.0f
        });

        const sf::FloatRect bounds = menuText[i].getLocalBounds();
        const float textWidth = bounds.size.x;
        const float arrowSpace = selected ? 34.0f : 0.0f;

        menuText[i].setPosition({
            (1280.0f - textWidth - arrowSpace) * 0.5f + arrowSpace,
            500.0f + static_cast<float>(i) * 55.0f
        });
    }
}

void TitleScreen::render(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);

    if (fontLoaded) {
        sf::Text title(font, "UNTITLED SPACE GAME", 38);
        title.setLetterSpacing(4.0f);
        title.setFillColor(sf::Color(245, 248, 255));

        const sf::FloatRect titleBounds = title.getLocalBounds();
        title.setOrigin({
            titleBounds.position.x + titleBounds.size.x * 0.5f,
            titleBounds.position.y + titleBounds.size.y * 0.5f
        });
        title.setPosition({640.0f, 76.0f});
        window.draw(title);

        sf::Text subtitle(font, "A 3D GAME", 13);
        subtitle.setLetterSpacing(2.0f);
        subtitle.setFillColor(sf::Color(105, 112, 125));

        const sf::FloatRect subtitleBounds = subtitle.getLocalBounds();
        subtitle.setOrigin({subtitleBounds.position.x + subtitleBounds.size.x * 0.5f, subtitleBounds.position.y + subtitleBounds.size.y * 0.5f});
        subtitle.setPosition({640.0f, 116.0f});
        window.draw(subtitle);
    }

    // Render the ship
    renderer.drawMesh(window, fighter.getMesh(), yaw, pitch, roll, {640.0f, 330.0f}, 0.82f, 3.0f);

    if (fontLoaded) {
        for (std::size_t i = 0; i < menuText.size(); ++i) {
            if (i == selectedOption) {
                sf::Text arrow(font, ">", 28);
                arrow.setFillColor(sf::Color::White);
                arrow.setPosition({
                    menuText[i].getPosition().x - 32.0f,
                    menuText[i].getPosition().y + 1.0f
                });
                window.draw(arrow);
            }

            window.draw(menuText[i]);
        }

        sf::Text instructions(font, "USE UP / DOWN TO SELECT    ENTER TO CONFIRM", 12);
        instructions.setLetterSpacing(1.0f);
        instructions.setFillColor(sf::Color(90, 95, 108));

        const sf::FloatRect bounds = instructions.getLocalBounds();
        instructions.setOrigin({
            bounds.position.x + bounds.size.x * 0.5f,
            bounds.position.y + bounds.size.y * 0.5f
        });
        instructions.setPosition({640.0f, 680.0f});
        window.draw(instructions);
    }

    window.display();
}

void TitleScreen::reset() {
    result = Result::None;
    selectedOption = 0;
    updateMenuText();
}

TitleScreen::Result TitleScreen::getResult() const {
    return result;
}
