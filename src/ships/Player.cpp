#include "ships/Player.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>

namespace {
constexpr float SHOT_COOLDOWN = 0.25f;
constexpr int SCORE_PER_HIT = 25;
}

Player::Player(float maximumHealth, float laserSpeed, float laserDamage)
        : maximumHealth(std::max(1.0f, maximumHealth)),
          health(std::max(1.0f, maximumHealth)),
          laserSpeed(std::max(0.0f, laserSpeed)),
          laserDamage(std::max(0.0f, laserDamage)) {
#ifdef SPACEGAME_FONT_PATH
    hudFontLoaded = hudFont.openFromFile(SPACEGAME_FONT_PATH);
#endif
    if (!hudFontLoaded) {
        hudFontLoaded = hudFont.openFromFile("src/font.ttf");
    }
    if (!hudFontLoaded) {
        hudFontLoaded = hudFont.openFromFile("../src/font.ttf");
    }
}

void Player::handleInput(float) {
}

void Player::reset() {
    orientation = Quaternion{};
    throttle = 0.0f;
    shotCooldown = 0.0f;
    health = maximumHealth;
    score = 0;
    hitShakeTimer = 0.0f;
    hitShakePhase = 0.0f;
    lasers.clear();
}

void Player::update(float deltaTime) {
    shotCooldown = std::max(0.0f, shotCooldown - deltaTime);
    hitShakeTimer = std::max(0.0f, hitShakeTimer - deltaTime);
    hitShakePhase += deltaTime * 90.0f;
}

void Player::updateLasers(float deltaTime, const Vector3& playerDisplacement, std::vector<std::unique_ptr<Enemy>>& enemies) {
    constexpr float ENEMY_HIT_RADIUS = 2.5f;

    for (const std::unique_ptr<Laser>& laser : lasers) {
        laser->update(deltaTime, playerDisplacement);
        if (laser->isExpired()) {
            continue;
        }

        for (const std::unique_ptr<Enemy>& enemy : enemies) {
            if (!enemy->isDestroyed() &&
                laser->intersectsSphere(enemy->getPosition(), ENEMY_HIT_RADIUS)) {
                onEnemyHit(*enemy, laser->getDamage());
                laser->consume();
                break;
            }
        }
    }

    lasers.erase(
        std::remove_if(
            lasers.begin(),
            lasers.end(),
            [](const std::unique_ptr<Laser>& laser) {
                return laser->isExpired();
            }
        ),
        lasers.end()
    );
}

void Player::yaw(float angle) {
    orientation.rotateLocal({0.0f, 1.0f, 0.0f}, angle);
}

void Player::pitch(float angle) {
    orientation.rotateLocal({1.0f, 0.0f, 0.0f}, angle);
}

void Player::roll(float angle) {
    orientation.rotateLocal({0.0f, 0.0f, 1.0f}, angle);
}

void Player::thrust(float amount) {
    throttle = std::clamp(throttle + amount, -1.0f, 1.0f);
}

void Player::shoot() {
}

bool Player::beginFiring() {
    if (shotCooldown > 0.0f) {
        return false;
    }

    shotCooldown = SHOT_COOLDOWN;
    return true;
}

void Player::fireLaser(const Vector3& origin, const Vector3& direction) {
    lasers.push_back(createLaser(origin, direction));
}

void Player::addScore(int points) {
    score += std::max(0, points);
}

void Player::createLaserPair() {
    if (!beginFiring()) {
        return;
    }

    constexpr float AIM_DISTANCE = 80.0f;
    constexpr float MUZZLE_OFFSET = 1.25f;
    const Vector3 target = orientation.rotateVector({0.0f, 0.0f, -AIM_DISTANCE});

    for (float side : {-1.0f, 1.0f}) {
        const Vector3 muzzle = orientation.rotateVector(
            {side * MUZZLE_OFFSET, -0.1f, -1.0f}
        );
        fireLaser(muzzle, target - muzzle);
    }
}

void Player::handleDefaultControls(float deltaTime) {
    constexpr float TURN_SPEED = 1.4f;
    constexpr float THROTTLE_CHANGE_RATE = 0.75f;
    const float turnAmount = TURN_SPEED * deltaTime;
    const float throttleAmount = THROTTLE_CHANGE_RATE * deltaTime;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        roll(turnAmount);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        roll(-turnAmount);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) {
        yaw(turnAmount);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) {
        yaw(-turnAmount);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
        pitch(-turnAmount);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
        pitch(turnAmount);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
        thrust(throttleAmount);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
        thrust(-throttleAmount);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
        shoot();
    }
}

void Player::onTakeDamage(float damage) {
    if (damage > 0 && health > 0) {
        health = std::max(0.0f, health - damage);
        hitShakeTimer = 0.24f;
    }
}

void Player::onEnemyHit(Enemy& enemy, float damage) {
    enemy.takeDamage(damage);
    addScore(SCORE_PER_HIT);
}

void Player::takeHit(float damage) {
    onTakeDamage(damage);
}

std::unique_ptr<Laser> Player::createLaser(const Vector3& origin, const Vector3& direction) const {
    return std::make_unique<Laser>(origin, direction, laserSpeed, laserDamage);
}

void Player::renderWeapons(const GpuRenderer& renderer) const {
    for (const std::unique_ptr<Laser>& laser : lasers) {
        renderer.drawLaser(laser->getTailPosition(), laser->getPosition(), sf::Color(90, 235, 255));
    }
}

void Player::renderHud(sf::RenderWindow& window) const {
    window.resetGLStates();
    window.setView(window.getDefaultView());

    constexpr float squareSize = 18.0f;
    constexpr float squareGap = 9.0f;
    constexpr float totalHealthWidth = 10.0f * squareSize + 9.0f * squareGap;
    const float firstSquareX =
        (static_cast<float>(window.getSize().x) - totalHealthWidth) * 0.5f;
    const float healthBarY = static_cast<float>(window.getSize().y) - 34.0f;

    if (hudFontLoaded) {
        sf::Text scoreText(hudFont, "SCORE " + std::to_string(score), 18);
        scoreText.setFillColor(sf::Color::White);
        const sf::FloatRect scoreBounds = scoreText.getLocalBounds();
        scoreText.setOrigin({
            scoreBounds.position.x + scoreBounds.size.x * 0.5f,
            scoreBounds.position.y + scoreBounds.size.y * 0.5f
        });
        scoreText.setPosition({
            static_cast<float>(window.getSize().x) * 0.5f,
            healthBarY - 22.0f
        });
        window.draw(scoreText);
    }

    const int activeSegments = static_cast<int>(std::ceil(
        health * 10.0f / maximumHealth
    ));
    for (int i = 0; i < activeSegments; ++i) {
        sf::RectangleShape healthSquare({squareSize, squareSize});
        healthSquare.setFillColor(sf::Color(235, 48, 58));
        healthSquare.setPosition({
            firstSquareX + static_cast<float>(i) * (squareSize + squareGap),
            healthBarY
        });
        window.draw(healthSquare);
    }
}

const Mesh& Player::getCockpitMesh() const {
    return cockpit.getMesh();
}

Vector3 Player::getCameraShakeOffset() const {
    constexpr float SHAKE_DURATION = 0.24f;
    const float envelope = hitShakeTimer / SHAKE_DURATION;
    return {
        std::sin(hitShakePhase) * 0.16f * envelope,
        std::sin(hitShakePhase * 1.37f) * 0.11f * envelope,
        0.0f
    };
}

const Quaternion& Player::getOrientation() const {
    return orientation;
}

float Player::getThrottle() const {
    return throttle;
}

float Player::getHealth() const {
    return health;
}

float Player::getMaximumHealth() const {
    return maximumHealth;
}

int Player::getScore() const {
    return score;
}

bool Player::isDestroyed() const {
    return health <= 0;
}