#pragma once

#include "TitleScreen.hpp"
#include "Graphics/GpuRenderer.hpp"
#include "Graphics/Starfield.hpp"
#include "Math/Quaternion.hpp"
#include "cockpit/Cockpit.hpp"
#include "ships/Enemy.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

#include <optional>
#include <random>
#include <vector>

class Game {
public:
    Game();
    void run();

private:
    enum class State {
        Title,
        Playing
    };

    sf::RenderWindow window;
    TitleScreen titleScreen;
    std::optional<Cockpit> cockpit;
    std::optional<Starfield> starfield;
    std::vector<Enemy> enemies;
    GpuRenderer gpuRenderer;
    Quaternion shipOrientation;
    float throttle{};
    float enemySpawnTimer{};
    std::mt19937 enemyRandom{1984};
    State state{State::Title};

    void spawnEnemy();
};
