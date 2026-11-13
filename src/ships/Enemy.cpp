#include "ships/Enemy.hpp"

#include "ships/Ship.hpp"

#include <algorithm>
#include <cmath>

namespace {
constexpr float MAX_SPEED = 38.0f;
constexpr float MAX_ACCELERATION = 16.0f;
constexpr float TURN_RATE = 1.05f;
constexpr float ROLL_RATE = 1.6f;
constexpr float THROTTLE_RESPONSE = 0.8f;
constexpr float ATTACK_RUN_DURATION = 3.5f;
constexpr float BREAK_TURN_DURATION = 4.5f;
constexpr float MIN_EVASION_DURATION = 2.5f;
constexpr float ENEMY_WEAPON_COOLDOWN = 1.25f;
constexpr float ENEMY_WEAPON_PROJECTILE_SPEED = 105.0f;
constexpr float ENEMY_WEAPON_RANGE = 105.0f;
constexpr float ENEMY_WEAPON_MIN_RANGE = 30.0f;
constexpr float ENEMY_WEAPON_AIM_DOT = 0.96f;

float dot(const Vector3& left, const Vector3& right) {
    return left.x * right.x + left.y * right.y + left.z * right.z;
}

float length(const Vector3& vector) {
    return std::sqrt(dot(vector, vector));
}

Vector3 normalized(const Vector3& vector) {
    const float vectorLength = length(vector);
    return vectorLength > 0.001f ? vector * (1.0f / vectorLength) : Vector3{};
}

Vector3 cross(const Vector3& left, const Vector3& right) {
    return {
        left.y * right.z - left.z * right.y,
        left.z * right.x - left.x * right.z,
        left.x * right.y - left.y * right.x
    };
}
}

Enemy::Enemy(const Vector3& position, float orbitRadius, float orbitAngle, float orbitSpeed, float verticalOffset, float maximumHealth)
    : position(position),
    health(std::max(1.0f, maximumHealth)),
      orbitRadius(orbitRadius),
      orbitAngle(orbitAngle),
      orbitSpeed(orbitSpeed),
    verticalOffset(verticalOffset) {
        attackRun = std::sin(orbitAngle) >= 0.0f;
        weaponCooldown = std::fmod(std::abs(orbitAngle), ENEMY_WEAPON_COOLDOWN);
}

void Enemy::update(float deltaTime, const Vector3& playerDisplacement, const Quaternion& playerOrientation) {
    weaponCooldown = std::max(0.0f, weaponCooldown - deltaTime);
    position = position - playerDisplacement;

    const float distanceToPlayer = length(position);
    const Vector3 outward = normalized(position);
    const Vector3 playerVelocity = deltaTime > 0.0f
        ? playerDisplacement * (1.0f / deltaTime)
        : Vector3{};
    const Vector3 playerForward = playerOrientation.rotateVector({0.0f, 0.0f, -1.0f});
    const float playerAim = std::clamp((dot(playerForward, outward) - 0.65f) / 0.30f, 0.0f, 1.0f);
    const float closingSpeed = -dot(outward, velocity - playerVelocity);
    const float closingFactor = std::clamp((closingSpeed + 2.0f) / 12.0f, 0.0f, 1.0f);
    const float rangeFactor = std::clamp((110.0f - distanceToPlayer) / 70.0f, 0.0f, 1.0f);
    const float incomingThreat = rangeFactor * playerAim * (0.65f + 0.35f * closingFactor);
    const float threatResponse = incomingThreat > threatLevel ? 2.8f : 0.65f;
    threatLevel += (incomingThreat - threatLevel) *
        std::min(1.0f, threatResponse * deltaTime);

    if (!evading && threatLevel >= 0.48f) {
        evading = true;
        evasionTimer = MIN_EVASION_DURATION;
        evasionPitchTimer = 0.0f;
        evasionDirection = orbitSpeed >= 0.0f ? 1.0f : -1.0f;
        evasionVerticalDirection = std::sin(orbitAngle * 2.7f) >= 0.0f ? 1.0f : -1.0f;
    }
    if (evading) {
        evasionTimer -= deltaTime;
        evasionPitchTimer += deltaTime;
        if (evasionPitchTimer >= 1.2f) {
            evasionPitchTimer -= 1.2f;
            evasionVerticalDirection *= -1.0f;
        }
        if (evasionTimer <= 0.0f && threatLevel < 0.20f) {
            evading = false;
        }
    }

    maneuverTimer += deltaTime;
    const float maneuverDuration = attackRun
        ? ATTACK_RUN_DURATION
        : BREAK_TURN_DURATION;
    if (maneuverTimer >= maneuverDuration) {
        maneuverTimer -= maneuverDuration;
        attackRun = !attackRun;
    }

    orbitAngle += orbitSpeed * deltaTime;

    const FlightCommand command = chooseFlightCommand({
        deltaTime,
        position,
        playerVelocity,
        playerOrientation,
        distanceToPlayer,
        orbitRadius,
        orbitAngle,
        orbitSpeed,
        verticalOffset,
        evading,
        attackRun
    });
    const Vector3 desiredCourse = command.course;
    float desiredThrottle = command.throttle;

    Vector3 localCourse = orientation.inverse().rotateVector(desiredCourse);
    const float yawError = std::atan2(localCourse.x, localCourse.z);
    const float turnRate = evading ? TURN_RATE * 1.45f : TURN_RATE;
    const float yawStep = std::clamp(yawError, -turnRate * deltaTime, turnRate * deltaTime);
    orientation.rotateLocal({0.0f, 1.0f, 0.0f}, yawStep);

    localCourse = orientation.inverse().rotateVector(desiredCourse);
    const float pitchError = std::atan2(-localCourse.y, std::sqrt(localCourse.x * localCourse.x + localCourse.z * localCourse.z));
    const float pitchStep = std::clamp(pitchError, -turnRate * deltaTime, turnRate * deltaTime);
    orientation.rotateLocal({1.0f, 0.0f, 0.0f}, pitchStep);

    const float currentRoll = std::atan2(2.0f * (orientation.w * orientation.z + orientation.x * orientation.y), 1.0f - 2.0f * (orientation.y * orientation.y + orientation.z * orientation.z));
    const float targetRoll = std::clamp(-yawError * 0.55f, -0.65f, 0.65f);
    const float rollStep = std::clamp(targetRoll - currentRoll, -ROLL_RATE * deltaTime, ROLL_RATE * deltaTime);
    orientation.rotateLocal({0.0f, 0.0f, 1.0f}, rollStep);

    desiredThrottle = std::clamp(desiredThrottle, 0.1f, 1.0f);
    throttle = std::clamp(throttle + std::clamp(desiredThrottle - throttle, -THROTTLE_RESPONSE * deltaTime, THROTTLE_RESPONSE * deltaTime), 0.1f, 1.0f);
    const Vector3 forward = orientation.rotateVector({0.0f, 0.0f, 1.0f});
    velocity = velocity + forward * (MAX_ACCELERATION * throttle * deltaTime);
    velocity = velocity * std::max(0.0f, 1.0f - 0.08f * deltaTime);
    const float speed = length(velocity);
    if (speed > MAX_SPEED) {
        velocity = velocity * (MAX_SPEED / speed);
    }
    position = position + velocity * deltaTime;
}

Enemy::FlightCommand Enemy::chooseFlightCommand(const FlightContext& context) const {
    if (context.evading) {
        return evadePlayer(context);
    }
    if (context.attackRun) {
        return attackPlayer(context);
    }
    return orbitPlayer(context);
}

Enemy::FlightCommand Enemy::evadePlayer(const FlightContext& context) const {
    const Vector3 outward = normalized(context.position);
    const Vector3 playerUp = context.playerOrientation.rotateVector({0.0f, 1.0f, 0.0f});
    const Vector3 lateral = normalized(cross(playerUp, outward)) * evasionDirection;
    const Vector3 course = normalized(lateral * 0.8f + outward * 0.4f + playerUp * (evasionVerticalDirection * 1.25f));
    return {course, context.distanceToPlayer < 24.0f ? 0.75f : 1.0f};
}

Enemy::FlightCommand Enemy::attackPlayer(const FlightContext& context) const {
    const float flightTime = context.distanceToPlayer / ENEMY_WEAPON_PROJECTILE_SPEED;
    const Vector3 leadPoint = context.playerVelocity * flightTime;
    const Vector3 course = normalized(leadPoint - context.position);
    const float throttle = context.distanceToPlayer > context.orbitRadius + 10.0f
        ? 1.0f
        : context.distanceToPlayer < 18.0f ? 0.2f : 0.35f;
    return {course, throttle};
}

Enemy::FlightCommand Enemy::orbitPlayer(const FlightContext& context) const {
    const Vector3 playerUp = context.playerOrientation.rotateVector({0.0f, 1.0f, 0.0f});
    const float altitude = dot(context.position, playerUp);
    const Vector3 horizontalPosition = context.position - playerUp * altitude;
    const Vector3 horizontalOutward = normalized(horizontalPosition);
    const float turnDirection = context.orbitSpeed >= 0.0f ? 1.0f : -1.0f;
    const Vector3 tangent = normalized(cross(playerUp, horizontalOutward)) * turnDirection;
    const float rangeError = length(horizontalPosition) - context.orbitRadius;
    const Vector3 radialCorrection = horizontalOutward * std::clamp(-rangeError * 0.025f, -0.45f, 0.45f);
    const float targetAltitude = context.verticalOffset +
        std::sin(context.orbitAngle * 0.65f) * context.orbitRadius * 0.5f;
    const Vector3 verticalCorrection = playerUp * std::clamp((targetAltitude - altitude) * 0.04f, -0.75f, 0.75f);
    const Vector3 course = normalized(tangent + radialCorrection + verticalCorrection);
    const float throttle = context.distanceToPlayer > context.orbitRadius + 10.0f
        ? 0.9f
        : 0.55f;
    return {course, throttle};
}

void Enemy::destroy() {
    destroyed = true;
}

void Enemy::takeDamage(float damage) {
    if (destroyed || damage <= 0.0f) {
        return;
    }

    health = std::max(0.0f, health - damage);
    if (health == 0.0f) {
        destroy();
    }
}

bool Enemy::tryShoot(const Vector3& playerVelocity) {
    if (!attackRun || evading || weaponCooldown > 0.0f) {
        return false;
    }

    const float distanceToPlayer = length(position);
    if (distanceToPlayer < ENEMY_WEAPON_MIN_RANGE ||
        distanceToPlayer > ENEMY_WEAPON_RANGE) {
        return false;
    }

    const Vector3 forward = orientation.rotateVector({0.0f, 0.0f, 1.0f});
    const float estimatedFlightTime = distanceToPlayer / ENEMY_WEAPON_PROJECTILE_SPEED;
    const Vector3 predictedTarget = position * -1.0f + playerVelocity * estimatedFlightTime;
    const Vector3 directionToTarget = normalized(predictedTarget);
    if (dot(forward, directionToTarget) < ENEMY_WEAPON_AIM_DOT) {
        return false;
    }

    weaponCooldown = ENEMY_WEAPON_COOLDOWN;
    return true;
}

float Enemy::getShotDamage() const {
    return 1.0f;
}

float Enemy::getShotSpeed() const {
    return ENEMY_WEAPON_PROJECTILE_SPEED;
}

bool Enemy::isDestroyed() const {
    return destroyed;
}

const Vector3& Enemy::getPosition() const {
    return position;
}

const Quaternion& Enemy::getOrientation() const {
    return orientation;
}

const Mesh& Enemy::getMesh() const {
    static const Ship shipModel;
    return shipModel.getMesh();
}