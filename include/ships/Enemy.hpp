#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "Math/Vector3.hpp"
#include "Graphics/Mesh.hpp"
#include "Math/Quaternion.hpp"

class Enemy {
public:
    Enemy(const Vector3& position, float orbitRadius, float orbitAngle, float orbitSpeed, float verticalOffset, float maximumHealth = 100.0f);
    virtual ~Enemy() = default;

    virtual void update(float deltaTime, const Vector3& playerDisplacement, const Quaternion& playerOrientation);
    virtual void destroy();
    virtual void takeDamage(float damage);
    virtual bool tryShoot(const Vector3& playerVelocity);
    virtual float getShotDamage() const;
    virtual float getShotSpeed() const;
    virtual bool isDestroyed() const;
    virtual const Vector3& getPosition() const;
    virtual const Quaternion& getOrientation() const;
    virtual const Mesh& getMesh() const;

protected:
    struct FlightContext {
        float deltaTime{};
        Vector3 position;
        Vector3 playerVelocity;
        Quaternion playerOrientation;
        float distanceToPlayer{};
        float orbitRadius{};
        float orbitAngle{};
        float orbitSpeed{};
        float verticalOffset{};
        bool evading{};
        bool attackRun{};
    };

    struct FlightCommand {
        Vector3 course;
        float throttle{};
    };

    virtual FlightCommand chooseFlightCommand(const FlightContext& context) const;
    FlightCommand evadePlayer(const FlightContext& context) const;
    FlightCommand attackPlayer(const FlightContext& context) const;
    FlightCommand orbitPlayer(const FlightContext& context) const;

private:
    Vector3 position;
    Vector3 velocity;
    Quaternion orientation;
    bool destroyed{};
    bool attackRun{};
    bool evading{};
    float throttle{0.65f};
    float health{100.0f};
    float threatLevel{};
    float evasionTimer{};
    float evasionPitchTimer{};
    float evasionDirection{1.0f};
    float evasionVerticalDirection{1.0f};
    float maneuverTimer{};
    float weaponCooldown{};
    float orbitRadius;
    float orbitAngle;
    float orbitSpeed;
    float verticalOffset;
};

#endif