#include "ships/Laser.hpp"

#include <algorithm>
#include <cmath>

namespace {
constexpr float LASER_LENGTH = 4.0f;

float dot(const Vector3& left, const Vector3& right) {
    return left.x * right.x + left.y * right.y + left.z * right.z;
}

float length(const Vector3& vector) {
    return std::sqrt(dot(vector, vector));
}
}

Laser::Laser(const Vector3& origin, const Vector3& direction, float speed, float damage)
    : previousPosition(origin), position(origin), damage(damage) {
    const float directionLength = length(direction);
    if (directionLength > 0.001f) {
        velocity = direction * (speed / directionLength);
    }
}

void Laser::update(float deltaTime, const Vector3& playerDisplacement) {
    previousPosition = position;
    position = position + velocity * deltaTime - playerDisplacement;
    lifetime -= deltaTime;
}

bool Laser::intersectsSphere(const Vector3& center, float radius) const {
    const Vector3 segment = position - previousPosition;
    const float segmentLengthSquared = dot(segment, segment);
    const float closestPointAmount = segmentLengthSquared > 0.0f
        ? std::clamp(dot(center - previousPosition, segment) / segmentLengthSquared, 0.0f, 1.0f)
        : 0.0f;
    const Vector3 closestPoint = previousPosition + segment * closestPointAmount;
    const Vector3 offset = center - closestPoint;
    return dot(offset, offset) <= radius * radius;
}

bool Laser::isExpired() const {
    return consumed || lifetime <= 0.0f;
}

void Laser::consume() {
    consumed = true;
}

float Laser::getDamage() const {
    return damage;
}

const Vector3& Laser::getPosition() const {
    return position;
}

Vector3 Laser::getTailPosition() const {
    const float speed = length(velocity);
    if (speed <= 0.001f) {
        return position;
    }

    return position - velocity * (LASER_LENGTH / speed);
}