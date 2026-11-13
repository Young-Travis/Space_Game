#pragma once

#include "GameOverScreen.hpp"
#include "TitleScreen.hpp"
#include "Graphics/GpuRenderer.hpp"
#include "Graphics/ParticleExplosion.hpp"
#include "Graphics/Starfield.hpp"
#include "Math/Quaternion.hpp"
#include "ships/Enemy.hpp"
#include "ships/Laser.hpp"
#include "ships/StudentPlayer.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

#include <cstddef>
#include <memory>
#include <random>
#include <vector>

class Game {
public:
    Game();
    void run();

private:
    enum class State {
        Title,
        Playing,
        GameOver
    };

    sf::RenderWindow window;
    TitleScreen titleScreen;
    GameOverScreen gameOverScreen;
    std::optional<Starfield> starfield;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<ParticleExplosion> explosions;
    std::vector<std::unique_ptr<Laser>> enemyLasers;
    GpuRenderer gpuRenderer;
    StudentPlayer player;
    float enemySpawnTimer{};
    float waveIntermissionTimer{};
    std::size_t enemiesRemainingInWave{4};
    std::mt19937 enemyRandom{1984};
    State state{State::Title};

    void spawnEnemy();
    void fireEnemyLasers(const Vector3& playerVelocity);
    void updateEnemyLasers(float deltaTime, const Vector3& playerDisplacement);
    void startGame();
};
