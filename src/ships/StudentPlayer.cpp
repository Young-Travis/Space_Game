#include "ships/StudentLaser.hpp"
#include "ships/StudentPlayer.hpp"

StudentPlayer::StudentPlayer(float maximumHealth)
    : Player(maximumHealth, StudentLaser::SPEED, StudentLaser::DAMAGE) {
}

void StudentPlayer::handleInput(float deltaTime) {
    Player::handleDefaultControls(deltaTime);
}

void StudentPlayer::shoot() {
    Player::createLaserPair();
}

void StudentPlayer::onTakeDamage(float damage) {
    Player::onTakeDamage(damage * DAMAGE_TAKEN_MULTIPLIER);
}

void StudentPlayer::onEnemyHit(Enemy& enemy, float damage) {
    enemy.takeDamage(damage);
    addScore(POINTS_PER_ENEMY_HIT);
}
