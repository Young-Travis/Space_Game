#ifndef STUDENT_LASER_HPP
#define STUDENT_LASER_HPP

#include "ships/Laser.hpp"

class StudentLaser : public Laser {
public:
    static constexpr float SPEED = 180.0f;
    static constexpr float DAMAGE = 25.0f;

    StudentLaser(const Vector3& origin, const Vector3& direction, float speed = SPEED, float damage = DAMAGE);
};

#endif