#include "Game.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <algorithm>
#include <random>

namespace {
constexpr float ENEMY_SPAWN_INTERVAL = 0.1f;
constexpr float ENEMY_LIFETIME = 30.0f;
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
            } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>();
                       keyPressed && keyPressed->code == sf::Keyboard::Key::Escape) {
                window.close();
            }
        }

        const float deltaTime = clock.restart().asSeconds();

        if (state == State::Title) {
            titleScreen.update(deltaTime);

            if (titleScreen.getResult() == TitleScreen::Result::Play) {
                cockpit.emplace();
                starfield.emplace();
                state = State::Playing;
            } else if (titleScreen.getResult() == TitleScreen::Result::Exit) {
                window.close();
            }
        }

        if (state == State::Playing) {
            constexpr float turnSpeed = 1.4f;
            constexpr float throttleChangeRate = 0.75f;
            const float turnAmount = turnSpeed * deltaTime;
            const float throttleAmount = throttleChangeRate * deltaTime;
            player.update(deltaTime);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
                player.roll(turnAmount);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
                player.roll(-turnAmount);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) {
                player.yaw(turnAmount);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) {
                player.yaw(-turnAmount);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
                player.pitch(-turnAmount);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
                player.pitch(turnAmount);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
                player.thrust(throttleAmount);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
                player.thrust(-throttleAmount);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
                static_cast<void>(player.shoot());
            }

            const Vector3 playerDisplacement = starfield->update(
                deltaTime,
                player.getThrottle(),
                player.getOrientation()
            );

            for (Enemy& enemy : enemies) {
                enemy.update(deltaTime, playerDisplacement);
            }
            enemies.erase(
                std::remove_if(
                    enemies.begin(),
                    enemies.end(),
                    [](const Enemy& enemy) {
                        return enemy.isExpired();
                    }
                ),
                enemies.end()
            );

            if (enemies.size() < MAX_ENEMIES) {
                enemySpawnTimer += deltaTime;
                while (enemySpawnTimer >= ENEMY_SPAWN_INTERVAL &&
                       enemies.size() < MAX_ENEMIES) {
                    enemySpawnTimer -= ENEMY_SPAWN_INTERVAL;
                    spawnEnemy();
                }
            } else {
                enemySpawnTimer = 0.0f;
            }
        }

        if (window.isOpen()) {
            if (state == State::Title) {
                titleScreen.render(window);
            } else {
                constexpr float cameraDistance = -0.30f;
                gpuRenderer.beginFrame(
                    window.getSize(),
                    cameraDistance
                );

                gpuRenderer.beginDepthPass();
                gpuRenderer.beginCockpitView(cameraDistance);
                gpuRenderer.drawMeshDepth(cockpit->getMesh());

                gpuRenderer.beginWorldView(cameraDistance, player.getOrientation());
                for (const Enemy& enemy : enemies) {
                    gpuRenderer.drawMeshDepth(
                        enemy.getMesh(),
                        enemy.getPosition()
                    );
                }

                gpuRenderer.beginColorPass();
                gpuRenderer.drawStars(starfield->getStars());

                for (const Enemy& enemy : enemies) {
                    gpuRenderer.drawMeshEdges(
                        enemy.getMesh(),
                        enemy.getPosition(),
                        sf::Color(100, 220, 255)
                    );
                }

                gpuRenderer.beginCockpitView(cameraDistance);
                gpuRenderer.drawMeshEdges(
                    cockpit->getMesh(),
                    {},
                    sf::Color::White
                );
                window.display();
            }
        }
    }
}

void Game::spawnEnemy() {
    std::uniform_real_distribution<float> horizontal(-20.0f, 20.0f);
    std::uniform_real_distribution<float> vertical(-12.0f, 12.0f);
    std::uniform_real_distribution<float> distance(65.0f, 100.0f);

    const Vector3 localPosition{
        horizontal(enemyRandom),
        vertical(enemyRandom),
        -distance(enemyRandom)
    };
    enemies.emplace_back(
        player.getOrientation().rotateVector(localPosition),
        ENEMY_LIFETIME
    );
}
