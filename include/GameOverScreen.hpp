#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>

#include <cstddef>

class GameOverScreen {
public:
    enum class Result {
        None,
        PlayAgain,
        MainMenu
    };

    GameOverScreen();

    void handleEvent(const sf::Event& event);
    void render(sf::RenderWindow& window);
    void reset();
    Result getResult() const;

private:
    sf::Font font;
    bool fontLoaded{};
    std::size_t selectedOption{};
    Result result{Result::None};
};