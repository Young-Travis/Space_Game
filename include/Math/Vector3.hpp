#ifndef VECTOR3_HPP
#define VECTOR3_HPP

#include <cmath>

struct Vector3 {
    float x{};
    float y{};
    float z{};

    constexpr Vector3 operator+(const Vector3& other) const {
        return {x + other.x, y + other.y, z + other.z};
    }

    constexpr Vector3 operator-(const Vector3& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }

    constexpr Vector3 operator*(float scalar) const {
        return {x * scalar, y * scalar, z * scalar};
    }
};

inline Vector3 rotateX(const Vector3& point, float angle) {
    const float c = std::cos(angle);
    const float s = std::sin(angle);

    return {
        point.x,
        point.y * c - point.z * s,
        point.y * s + point.z * c
    };
}

inline Vector3 rotateY(const Vector3& point, float angle) {
    const float c = std::cos(angle);
    const float s = std::sin(angle);

    return {
        point.x * c + point.z * s,
        point.y,
        -point.x * s + point.z * c
    };
}

inline Vector3 rotateZ(const Vector3& point, float angle) {
    const float c = std::cos(angle);
    const float s = std::sin(angle);

    return {
        point.x * c - point.y * s,
        point.x * s + point.y * c,
        point.z
    };
}

#endif
