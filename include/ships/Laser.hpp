#ifndef LASER_HPP
#define LASER_HPP

#include "Math/Vector3.hpp"

class Laser {
public:
    static constexpr float DAMAGE = 25.0f;

    Laser(const Vector3& origin, const Vector3& direction, float speed = 180.0f, float damage = DAMAGE);
    virtual ~Laser() = default;

    virtual void update(float deltaTime, const Vector3& playerDisplacement);
    virtual bool intersectsSphere(const Vector3& center, float radius) const;
    virtual bool isExpired() const;
    virtual void consume();
    virtual float getDamage() const;

    virtual const Vector3& getPosition() const;
    virtual Vector3 getTailPosition() const;

private:
    Vector3 previousPosition;
    Vector3 position;
    Vector3 velocity;
    float damage{DAMAGE};
    float lifetime{1.5f};
    bool consumed{};
};

#endif