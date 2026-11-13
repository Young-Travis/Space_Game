#include "Game.hpp"
#include "ships/StudentEnemy.hpp"
#include "ships/StudentLaser.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <algorithm>
#include <random>

namespace {
constexpr float ENEMY_SPAWN_INTERVAL = 1.5f;
constexpr float WAVE_INTERMISSION = 5.0f;
constexpr std::size_t MAX_ENEMIES = 4;

sf::ContextSettings createContextSettings() {
    sf::ContextSettings settings;
    settings.depthBits = 24;
    return settings;
}
}

Game::Game()
    : window(
        sf::VideoMode({1280u, 720u}),
        "Test Game (WIP)",
        sf::Style::Titlebar | sf::Style::Close,
        sf::State::Windowed,
        createContextSettings()
    ) {
    window.setVerticalSyncEnabled(true);
    window.setKeyRepeatEnabled(false);
}

void Game::run() {
    sf::Clock clock;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (state == State::Title) {
                titleScreen.handleEvent(*event);
            } else if (state == State::GameOver) {
                gameOverScreen.handleEvent(*event);
            } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>();
                       keyPressed && keyPressed->code == sf::Keyboard::Key::Escape) {
                window.close();
            }
        }

        if (state == State::GameOver) {
            const GameOverScreen::Result result = gameOverScreen.getResult();
            if (result == GameOverScreen::Result::PlayAgain) {
                startGame();
            } else if (result == GameOverScreen::Result::MainMenu) {
                titleScreen.reset();
                gameOverScreen.reset();
                enemies.clear();
                enemyLasers.clear();
                state = State::Title;
            }
        }

        const float deltaTime = clock.restart().asSeconds();

        if (state == State::Title) {
            titleScreen.update(deltaTime);

            if (titleScreen.getResult() == TitleScreen::Result::Play) {
                startGame();
            } else if (titleScreen.getResult() == TitleScreen::Result::Exit) {
                window.close();
            }
        }

        if (state == State::Playing) {
            player.update(deltaTime);
            player.handleInput(deltaTime);

            const Vector3 playerDisplacement = starfield->update(
                deltaTime,
                player.getThrottle(),
                player.getOrientation()
            );
            for (const std::unique_ptr<Enemy>& enemy : enemies) {
                enemy->update(
                    deltaTime,
                    playerDisplacement,
                    player.getOrientation()
                );
            }
            const Vector3 playerVelocity = deltaTime > 0.0f
                ? playerDisplacement * (1.0f / deltaTime)
                : Vector3{};
            fireEnemyLasers(playerVelocity);
            player.updateLasers(deltaTime, playerDisplacement, enemies);
            updateEnemyLasers(deltaTime, playerDisplacement);
            for (const std::unique_ptr<Enemy>& enemy : enemies) {
                if (enemy->isDestroyed()) {
                    explosions.emplace_back(enemy->getPosition());
                }
            }
            for (ParticleExplosion& explosion : explosions) {
                explosion.update(deltaTime, playerDisplacement);
            }
            explosions.erase(
                std::remove_if(
                    explosions.begin(),
                    explosions.end(),
                    [](const ParticleExplosion& explosion) {
                        return explosion.isExpired();
                    }
                ),
                explosions.end()
            );
            if (player.isDestroyed()) {
                state = State::GameOver;
            }
            enemies.erase(
                std::remove_if(
                    enemies.begin(),
                    enemies.end(),
                    [](const std::unique_ptr<Enemy>& enemy) {
                        return enemy->isDestroyed();
                    }
                ),
                enemies.end()
            );

            if (enemiesRemainingInWave > 0 && enemies.size() < MAX_ENEMIES) {
                enemySpawnTimer += deltaTime;
                if (enemySpawnTimer >= ENEMY_SPAWN_INTERVAL) {
                    enemySpawnTimer = 0.0f;
                    spawnEnemy();
                    --enemiesRemainingInWave;
                }
            } else if (enemiesRemainingInWave == 0 && enemies.empty()) {
                waveIntermissionTimer += deltaTime;
                if (waveIntermissionTimer >= WAVE_INTERMISSION) {
                    waveIntermissionTimer = 0.0f;
                    enemiesRemainingInWave = MAX_ENEMIES;
                }
            } else if (enemies.size() >= MAX_ENEMIES) {
                enemySpawnTimer = 0.0f;
            }
        }

        if (window.isOpen()) {
            if (state == State::Title) {
                titleScreen.render(window);
            } else if (state == State::GameOver) {
                gameOverScreen.render(window);
            } else {
                constexpr float cameraDistance = -0.30f;
                const Vector3 cameraShake = player.getCameraShakeOffset();
                gpuRenderer.beginFrame(window.getSize(), cameraDistance);

                gpuRenderer.beginDepthPass();
                gpuRenderer.beginCockpitView(cameraDistance, cameraShake);
                gpuRenderer.drawMeshDepth(player.getCockpitMesh());

                gpuRenderer.beginWorldView(cameraDistance, player.getOrientation(), cameraShake);
                for (const std::unique_ptr<Enemy>& enemy : enemies) {
                    gpuRenderer.drawMeshDepth(enemy->getMesh(), enemy->getPosition(), enemy->getOrientation());
                }

                gpuRenderer.beginColorPass();
                gpuRenderer.drawStars(starfield->getStars());
                for (const ParticleExplosion& explosion : explosions) {
                    gpuRenderer.drawExplosionParticles(explosion.getParticles());
                }

                for (const std::unique_ptr<Enemy>& enemy : enemies) {
                    gpuRenderer.drawMeshEdges(enemy->getMesh(), enemy->getPosition(), sf::Color::White, enemy->getOrientation());
                }
                player.renderWeapons(gpuRenderer);
                for (const std::unique_ptr<Laser>& laser : enemyLasers) {
                    gpuRenderer.drawLaser(laser->getTailPosition(), laser->getPosition(), sf::Color(255, 95, 70));
                }

                gpuRenderer.beginCockpitView(cameraDistance, cameraShake);
                gpuRenderer.drawMeshEdges(player.getCockpitMesh(), {}, sf::Color::White);

                player.renderHud(window);
                window.display();
            }
        }
    }
}

void Game::spawnEnemy() {
    std::uniform_real_distribution<float> horizontal(-24.0f, 24.0f);
    std::uniform_real_distribution<float> vertical(-16.0f, 16.0f);
    std::uniform_real_distribution<float> distance(55.0f, 80.0f);
    std::uniform_real_distribution<float> orbitRadius(24.0f, 38.0f);
    std::uniform_real_distribution<float> orbitAngle(0.0f, 6.28318530718f);
    std::uniform_real_distribution<float> orbitSpeed(0.25f, 0.55f);
    std::uniform_real_distribution<float> direction(0.0f, 1.0f);

    const Vector3 localPosition{
        horizontal(enemyRandom),
        vertical(enemyRandom),
        -distance(enemyRandom)
    };
    enemies.push_back(std::make_unique<StudentEnemy>(player.getOrientation().rotateVector(localPosition), orbitRadius(enemyRandom), orbitAngle(enemyRandom), orbitSpeed(enemyRandom) * (direction(enemyRandom) < 0.5f ? -1.0f : 1.0f), vertical(enemyRandom)));
}

void Game::fireEnemyLasers(const Vector3& playerVelocity) {
    for (const std::unique_ptr<Enemy>& enemy : enemies) {
        if (!enemy->tryShoot(playerVelocity)) {
            continue;
        }

        const Vector3 firingDirection = enemy->getOrientation().rotateVector(
            {0.0f, 0.0f, 1.0f}
        );
        const Vector3 muzzle = enemy->getPosition() +
            enemy->getOrientation().rotateVector({0.0f, 0.0f, 2.0f});
        enemyLasers.push_back(std::make_unique<StudentLaser>(muzzle, firingDirection, enemy->getShotSpeed(), enemy->getShotDamage()));
    }
}

void Game::updateEnemyLasers(float deltaTime, const Vector3& playerDisplacement) {
    constexpr float PLAYER_HIT_RADIUS = 2.0f;

    for (const std::unique_ptr<Laser>& laser : enemyLasers) {
        laser->update(deltaTime, playerDisplacement);
        if (!laser->isExpired() && laser->intersectsSphere({}, PLAYER_HIT_RADIUS)) {
            player.takeHit(laser->getDamage());
            laser->consume();
        }
    }

    enemyLasers.erase(
        std::remove_if(
            enemyLasers.begin(),
            enemyLasers.end(),
            [](const std::unique_ptr<Laser>& laser) {
                return laser->isExpired();
            }
        ),
        enemyLasers.end()
    );
}

void Game::startGame() {
    player.reset();
    enemies.clear();
    explosions.clear();
    enemyLasers.clear();
    starfield.emplace();
    enemySpawnTimer = ENEMY_SPAWN_INTERVAL;
    waveIntermissionTimer = 0.0f;
    enemiesRemainingInWave = MAX_ENEMIES;
    titleScreen.reset();
    gameOverScreen.reset();
    state = State::Playing;
}
