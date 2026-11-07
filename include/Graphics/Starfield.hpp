#ifndef STARFIELD_HPP
#define STARFIELD_HPP

#include "Graphics/Renderer3D.hpp"
#include "Math/Quaternion.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

#include <random>
#include <vector>

class Starfield {
public:
    Starfield();

    Vector3 update(float deltaTime, float throttle, const Quaternion& shipOrientation);

    const std::vector<Vector3>& getStars() const;

    void render(
        sf::RenderTarget& target,
        const Renderer3D& renderer,
        const Renderer3D::DepthBuffer& depthBuffer,
        const sf::Vector2f& screenCenter,
        float scale,
        float cameraDistance,
        const Quaternion& shipOrientation
    ) const;

private:
    Vector3 createStar(const Quaternion& shipOrientation, bool aheadOfShip);

    std::vector<Vector3> stars;
    std::mt19937 randomEngine;
};

#endif