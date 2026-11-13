#ifndef STUDENT_PLAYER_HPP
#define STUDENT_PLAYER_HPP

#include "ships/Player.hpp"

class StudentPlayer : public Player {
public:
    static constexpr float MAX_HEALTH = 10.0f;
    static constexpr float DAMAGE_TAKEN_MULTIPLIER = 1.0f;
    static constexpr int POINTS_PER_ENEMY_HIT = 25;

    explicit StudentPlayer(float maximumHealth = MAX_HEALTH);

    void handleInput(float deltaTime) override;
    void shoot() override;

protected:
    void onTakeDamage(float damage) override;
    void onEnemyHit(Enemy& enemy, float damage) override;
};

#endif