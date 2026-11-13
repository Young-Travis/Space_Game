#include "ships/Player.hpp"

#include <algorithm>

namespace {
constexpr float SHOT_COOLDOWN = 0.25f;
}

void Player::update(float deltaTime) {
    shotCooldown = std::max(0.0f, shotCooldown - deltaTime);
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

bool Player::shoot() {
    if (shotCooldown > 0.0f) {
        return false;
    }

    shotCooldown = SHOT_COOLDOWN;
    return true;
}

const Quaternion& Player::getOrientation() const {
    return orientation;
}

float Player::getThrottle() const {
    return throttle;
}