#include "ships/Enemy.hpp"

#include "ships/Ship.hpp"

Enemy::Enemy(const Vector3& position, float lifetime)
    : position(position), lifetime(lifetime) {
}

void Enemy::update(float deltaTime, const Vector3& playerDisplacement) {
    position = position - playerDisplacement;
    lifetime -= deltaTime;
}

bool Enemy::isExpired() const {
    return lifetime <= 0.0f;
}

const Vector3& Enemy::getPosition() const {
    return position;
}

const Mesh& Enemy::getMesh() const {
    static const Ship shipModel;
    return shipModel.getMesh();
}