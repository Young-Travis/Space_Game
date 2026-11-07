#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "Math/Vector3.hpp"
#include "Graphics/Mesh.hpp"

class Enemy {
public:
    Enemy(const Vector3& position, float lifetime);

    void update(float deltaTime, const Vector3& playerDisplacement);
    bool isExpired() const;
    const Vector3& getPosition() const;
    const Mesh& getMesh() const;

private:
    Vector3 position;
    float lifetime;
};

#endif