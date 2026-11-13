#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Math/Quaternion.hpp"

class Player {
public:
    void update(float deltaTime);

    void yaw(float angle);
    void pitch(float angle);
    void roll(float angle);
    void thrust(float amount);
    bool shoot();

    const Quaternion& getOrientation() const;
    float getThrottle() const;

private:
    Quaternion orientation;
    float throttle{};
    float shotCooldown{};
};

#endif