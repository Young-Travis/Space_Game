#pragma once

#include "Graphics/Mesh.hpp"
#include "Graphics/Renderer3D.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include "ships/Ship.hpp"

#include <vector>
#include <array>
#include <cstddef>

class TitleScreen {
public:
    enum class Result {
        None,
        Play,
        Exit
    };

    TitleScreen();

    void handleEvent(const sf::Event& event);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);
    void reset();

    Result getResult() const;

private:
    enum class MenuOption : std::size_t {
        Play = 0,
        Exit = 1
    };

    void moveSelection(int direction);
    void updateMenuText();

    Ship fighter;
    Renderer3D renderer;

    sf::Font font;
    bool fontLoaded{false};

    std::vector<sf::Text> menuText;
    std::size_t selectedOption{0};

    float yaw{0.0f};
    float pitch{0.0f};
    float roll{0.0f};

    Result result{Result::None};
};
