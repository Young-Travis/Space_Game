#include "Graphics/ParticleExplosion.hpp"

#include <algorithm>
#include <cmath>
#include <random>

namespace {
constexpr std::size_t PARTICLE_COUNT = 72;

float length(const Vector3& vector) {
    return std::sqrt(
        vector.x * vector.x +
        vector.y * vector.y +
        vector.z * vector.z
    );
}
}

ParticleExplosion::ParticleExplosion(const Vector3& origin) {
    static std::mt19937 randomEngine{std::random_device{}()};
    std::uniform_real_distribution<float> component(-1.0f, 1.0f);
    std::uniform_real_distribution<float> speed(9.0f, 28.0f);
    std::uniform_real_distribution<float> lifetime(0.65f, 1.2f);

    particles.reserve(PARTICLE_COUNT);
    while (particles.size() < PARTICLE_COUNT) {
        Vector3 direction{component(randomEngine), component(randomEngine), component(randomEngine)};
        const float directionLength = length(direction);
        if (directionLength < 0.1f) {
            continue;
        }

        direction = direction * (1.0f / directionLength);
        const float particleLifetime = lifetime(randomEngine);
        particles.push_back({
            origin,
            direction * speed(randomEngine),
            particleLifetime,
            particleLifetime
        });
    }
}

void ParticleExplosion::update(float deltaTime, const Vector3& playerDisplacement) {
    for (ExplosionParticle& particle : particles) {
        particle.position = particle.position + particle.velocity * deltaTime - playerDisplacement;
        particle.velocity = particle.velocity * std::max(0.0f, 1.0f - 0.8f * deltaTime);
        particle.lifetime -= deltaTime;
    }
}

bool ParticleExplosion::isExpired() const {
    return std::all_of(particles.begin(), particles.end(), [](const ExplosionParticle& particle) {return particle.lifetime <= 0.0f;});
}

const std::vector<ExplosionParticle>& ParticleExplosion::getParticles() const {
    return particles;
}