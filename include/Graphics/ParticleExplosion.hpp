#ifndef PARTICLE_EXPLOSION_HPP
#define PARTICLE_EXPLOSION_HPP

#include "Math/Vector3.hpp"

#include <vector>

struct ExplosionParticle {
    Vector3 position;
    Vector3 velocity;
    float lifetime{};
    float maxLifetime{};
};

class ParticleExplosion {
public:
    explicit ParticleExplosion(const Vector3& origin);

    void update(float deltaTime, const Vector3& playerDisplacement);
    bool isExpired() const;
    const std::vector<ExplosionParticle>& getParticles() const;

private:
    std::vector<ExplosionParticle> particles;
};

#endif