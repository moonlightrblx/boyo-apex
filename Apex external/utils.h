


#include "Windows.h"
#include "iostream"
#include "thread"



struct Vector3 {
    float x, y, z;

    float Length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    float Distance(const Vector3& other) const {
        return (*this - other).Length();
    }

    Vector3 operator-(const Vector3& other) const {
        return { x - other.x, y - other.y, z - other.z };
    }

    Vector3 operator+(const Vector3& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }

    Vector3& operator+=(const Vector3& other) {
        x += other.x; y += other.y; z += other.z;
        return *this;
    }

    Vector3& operator-=(const Vector3& other) {
        x -= other.x; y -= other.y; z -= other.z;
        return *this;
    }

    Vector3 operator*(float scalar) const {
        return { x * scalar, y * scalar, z * scalar };
    }

    Vector3& operator*=(float scalar) {
        x *= scalar; y *= scalar; z *= scalar;
        return *this;
    }
};

template <typename T>
constexpr const T& clamp(const T& value, const T& min, const T& max) {
    return (value < min) ? min : (value > max ? max : value);
}

struct Matrix {
    float matrix[16];
};

