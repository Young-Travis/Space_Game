#ifndef QUATERNION_HPP
#define QUATERNION_HPP

#include "Math/Vector3.hpp"

#include <cmath>

struct Quaternion {
    float w{1.0f};
    float x{};
    float y{};
    float z{};

    static Quaternion fromAxisAngle(const Vector3& axis, float angle) {
        const float axisLength = std::sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
        if (axisLength == 0.0f) {
            return {};
        }

        const float halfAngle = angle * 0.5f;
        const float axisScale = std::sin(halfAngle) / axisLength;
        return {
            std::cos(halfAngle),
            axis.x * axisScale,
            axis.y * axisScale,
            axis.z * axisScale
        };
    }

    Quaternion operator*(const Quaternion& other) const {
        return {
            w * other.w - x * other.x - y * other.y - z * other.z,
            w * other.x + x * other.w + y * other.z - z * other.y,
            w * other.y - x * other.z + y * other.w + z * other.x,
            w * other.z + x * other.y - y * other.x + z * other.w
        };
    }

    Quaternion normalized() const {
        const float length = std::sqrt(w * w + x * x + y * y + z * z);
        if (length == 0.0f) {
            return {};
        }

        return {w / length, x / length, y / length, z / length};
    }

    Quaternion conjugate() const {
        return {w, -x, -y, -z};
    }

    Quaternion inverse() const {
        return normalized().conjugate();
    }

    Vector3 rotateVector(const Vector3& vector) const {
        const Quaternion unit = normalized();
        const Quaternion pureVector{0.0f, vector.x, vector.y, vector.z};
        const Quaternion rotated = unit * pureVector * unit.conjugate();
        return {rotated.x, rotated.y, rotated.z};
    }

    void rotateLocal(const Vector3& axis, float angle) {
        *this = (*this * fromAxisAngle(axis, angle)).normalized();
    }
};

#endif