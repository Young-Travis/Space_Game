#ifndef GPU_RENDERER_HPP
#define GPU_RENDERER_HPP

#include "Graphics/Mesh.hpp"
#include "Graphics/ParticleExplosion.hpp"
#include "Math/Quaternion.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

#include <vector>

class GpuRenderer {
public:
    void beginFrame(const sf::Vector2u& targetSize, float cameraDistance) const;

    void beginWorldView(float cameraDistance, const Quaternion& shipOrientation, const Vector3& cameraShake = {}) const;
    void beginCockpitView(float cameraDistance, const Vector3& cameraShake = {}) const;
    void beginDepthPass() const;
    void drawMeshDepth(const Mesh& mesh, const Vector3& translation = {}, const Quaternion& orientation = {}) const;
    void beginColorPass() const;
    void drawStars(const std::vector<Vector3>& stars) const;
    void drawLaser(const Vector3& start, const Vector3& end, sf::Color color) const;
    void drawExplosionParticles(const std::vector<ExplosionParticle>& particles) const;
    void drawMeshEdges(const Mesh& mesh, const Vector3& translation = {}, sf::Color color = sf::Color::White, const Quaternion& orientation = {}) const;
};

#endif