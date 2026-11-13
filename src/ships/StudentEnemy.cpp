#include "ships/StudentEnemy.hpp"

StudentEnemy::StudentEnemy(const Vector3& position, float orbitRadius, float orbitAngle, float orbitSpeed, float verticalOffset)
    : Enemy(position, orbitRadius, orbitAngle, orbitSpeed, verticalOffset, MAX_HEALTH) {
}

float StudentEnemy::getShotDamage() const {
    return LASER_DAMAGE;
}

float StudentEnemy::getShotSpeed() const {
    return LASER_SPEED;
}

Enemy::FlightCommand StudentEnemy::chooseFlightCommand(const FlightContext& context) const {
    if (context.evading) {
        return evadePlayer(context);
    }
    if (context.attackRun) {
        return attackPlayer(context);
    }
    return orbitPlayer(context);
}