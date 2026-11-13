#ifndef STUDENT_ENEMY_HPP
#define STUDENT_ENEMY_HPP

#include "ships/Enemy.hpp"

class StudentEnemy : public Enemy {
public:
    static constexpr float MAX_HEALTH = 100.0f;
    static constexpr float LASER_DAMAGE = 1.0f;
    static constexpr float LASER_SPEED = 105.0f;

    StudentEnemy(const Vector3& position, float orbitRadius, float orbitAngle, float orbitSpeed, float verticalOffset);

    float getShotDamage() const override;
    float getShotSpeed() const override;

protected:
    FlightCommand chooseFlightCommand(const FlightContext& context) const override;
};

#endif