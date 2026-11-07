#ifndef RENDERER3D_HPP
#define RENDERER3D_HPP

#include "Graphics/Mesh.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

#include <vector>

class Renderer3D {
public:
    struct ProjectedPoint {
        sf::Vector2f position;
        float depth{};
        bool visible{};
    };

    struct DepthBuffer {
        unsigned int width{};
        unsigned int height{};
        std::vector<float> reciprocalDepth;

        bool isPointVisible(const sf::Vector2f& position, float depth) const;
    };

    Renderer3D(unsigned int screenWidth, unsigned int screenHeight);

    DepthBuffer buildDepthBuffer(
        const sf::RenderTarget& target,
        const Mesh& mesh,
        float yaw,
        float pitch,
        float roll,
        const sf::Vector2f& screenCenter,
        float scale,
        float cameraDistance,
        const Vector3& translation = {}
    ) const;

    void drawMeshEdges(
        sf::RenderTarget& target,
        const Mesh& mesh,
        float yaw,
        float pitch,
        float roll,
        const sf::Vector2f& screenCenter,
        float scale,
        float cameraDistance,
        const DepthBuffer& depthBuffer,
        const Vector3& translation = {}
    ) const;

    void drawMesh(
        sf::RenderTarget& target,
        const Mesh& mesh,
        float yaw,
        float pitch,
        float roll,
        const sf::Vector2f& screenCenter,
        float scale,
        float cameraDistance,
        const Vector3& translation = {}
    ) const;

    ProjectedPoint projectPoint(
        const Vector3& point,
        const sf::Vector2f& screenCenter,
        float scale,
        float cameraDistance
    ) const;

private:
    ProjectedPoint project(
        const Vector3& point,
        const sf::Vector2f& screenCenter,
        float scale,
        float cameraDistance
    ) const;

    float focalLength;
    float nearPlane;
};

#endif