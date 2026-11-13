#include "Graphics/Starfield.hpp"

#include <SFML/Graphics/Vertex.hpp>

#include <cmath>
#include <random>

namespace {
constexpr std::size_t STAR_COUNT = 700;
constexpr float MAX_STAR_SPEED = 30.0f;
constexpr float STAR_FIELD_DISTANCE = 180.0f;
constexpr float MIN_STAR_DISTANCE = 25.0f;
constexpr float TWO_PI = 6.28318530718f;
}

Starfield::Starfield() : randomEngine(1984) {
    stars.reserve(STAR_COUNT);
    for (std::size_t i = 0; i < STAR_COUNT; ++i) {
        stars.push_back(createStar(Quaternion{}, false));
    }
}

const std::vector<Vector3>& Starfield::getStars() const {
    return stars;
}

Vector3 Starfield::createStar(const Quaternion& shipOrientation, bool aheadOfShip) {
    const float minimumZDirection = aheadOfShip ? -1.0f : -1.0f;
    const float maximumZDirection = aheadOfShip ? -0.5f : 1.0f;
    std::uniform_real_distribution<float> zDirection(minimumZDirection, maximumZDirection);
    std::uniform_real_distribution<float> azimuth(0.0f, TWO_PI);
    std::uniform_real_distribution<float> radius(90.0f, 150.0f);

    const float z = zDirection(randomEngine);
    const float angle = azimuth(randomEngine);
    const float horizontalRadius = std::sqrt(1.0f - z * z);
    const float distance = radius(randomEngine);

    Vector3 star{
        distance * horizontalRadius * std::cos(angle),
        distance * horizontalRadius * std::sin(angle),
        distance * z
    };

    if (aheadOfShip) {
        star = shipOrientation.rotateVector(star);
    }

    return star;
}

Vector3 Starfield::update(float deltaTime, float throttle, const Quaternion& shipOrientation) {
    const Vector3 forward = shipOrientation.rotateVector({0.0f, 0.0f, -1.0f});
    const Vector3 displacement = forward * (throttle * MAX_STAR_SPEED * deltaTime);

    for (Vector3& star : stars) {
        star = star - displacement;

        const float forwardDistance =
            star.x * forward.x +
            star.y * forward.y +
            star.z * forward.z;
        const float distanceSquared =
            star.x * star.x + star.y * star.y + star.z * star.z;

        if (forwardDistance < -STAR_FIELD_DISTANCE ||
            distanceSquared < MIN_STAR_DISTANCE * MIN_STAR_DISTANCE) {
            star = createStar(shipOrientation, true);
        }
    }

    return displacement;
}

void Starfield::render(sf::RenderTarget& target, const Renderer3D& renderer, const Renderer3D::DepthBuffer& depthBuffer, const sf::Vector2f& screenCenter, float scale, float cameraDistance, const Quaternion& shipOrientation) const {
    std::vector<sf::Vertex> visibleStars;
    visibleStars.reserve(stars.size());

    for (const Vector3& star : stars) {
        const Vector3 viewStar = shipOrientation.inverse().rotateVector(star);
        const Renderer3D::ProjectedPoint projected = renderer.projectPoint(viewStar, screenCenter, scale, cameraDistance);

        if (!projected.visible ||
            !depthBuffer.isPointVisible(projected.position, projected.depth)) {
            continue;
        }

        sf::Vertex vertex;
        vertex.position = projected.position;
        vertex.color = sf::Color(220, 230, 245);
        visibleStars.push_back(vertex);
    }

    if (!visibleStars.empty()) {
        target.draw(visibleStars.data(), visibleStars.size(), sf::PrimitiveType::Points);
    }
}