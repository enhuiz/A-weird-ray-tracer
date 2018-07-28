#ifndef VECTOR3_HPP
#define VECTOR3_HPP

#include <array>
#include <algorithm>
#include <limits>
#include <string>
#include <sstream>
#include <ostream>
#include <cmath>
#include <cassert>
#include <type_traits>

template <typename T>
struct Vector3
{
    union {
        struct
        {
            T x, y, z;
        };
        std::array<T, 3> components;
    };

    Vector3() {}
    Vector3(T x, T y, T z) : x(x), y(y), z(z) {}
    explicit Vector3(const T &scalar) : x(scalar), y(scalar), z(scalar) {}

    // allow copy
    Vector3(const Vector3<T> &) = default;
    Vector3 &operator=(const Vector3<T> &) = default;

    // Array like element access
    T &element(std::size_t index) { return components[index]; }
    const T &element(std::size_t index) const { return components[index]; }

    T &operator[](std::size_t index) { return element(index); }
    const T &operator[](std::size_t index) const { return element(index); }

    // Mutable operations
    template <typename VectorType>
    Vector3<T> &operator+=(VectorType &&other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    template <typename VectorType>
    Vector3<T> &operator-=(VectorType &&other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    template <typename VectorType>
    auto operator*=(VectorType &&other)
        -> typename std::enable_if<!std::is_scalar<typename std::decay<decltype(other)>::type>::value, Vector3<T> &>::type
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    template <typename VectorType>
    auto operator/=(VectorType &&other)
        -> typename std::enable_if<!std::is_scalar<typename std::decay<decltype(other)>::type>::value, Vector3<T> &>::type
    {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        return *this;
    }

    template <typename ScalarType>
    auto operator*=(ScalarType &&scalar)
        -> typename std::enable_if<std::is_scalar<typename std::decay<decltype(scalar)>::type>::value, Vector3<T> &>::type
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    template <typename ScalarType>
    auto operator/=(ScalarType &&scalar)
        -> typename std::enable_if<std::is_scalar<typename std::decay<decltype(scalar)>::type>::value, Vector3<T> &>::type
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    // immutable operations
    template <typename VectorType>
    Vector3<T> operator+(VectorType &&other) const
    {
        return {x + other.x, y + other.y, z + other.z};
    }

    template <typename VectorType>
    Vector3<T> operator-(VectorType &&other) const
    {
        return {x - other.x, y - other.y, z - other.z};
    }

    template <typename VectorType>
    auto operator*(VectorType &&other) const
        -> typename std::enable_if<!std::is_scalar<typename std::decay<decltype(other)>::type>::value, Vector3<T>>::type
    {
        return {x * other.x, y * other.y, z * other.z};
    }

    template <typename VectorType>
    auto operator/(VectorType &&other) const
        -> typename std::enable_if<!std::is_scalar<typename std::decay<decltype(other)>::type>::value, Vector3<T>>::type
    {
        return {x / other.x, y / other.y, z / other.z};
    }

    template <typename ScalarType>
    auto operator*(ScalarType &&scalar) const
        -> typename std::enable_if<std::is_scalar<typename std::decay<decltype(scalar)>::type>::value, Vector3<T>>::type
    {
        return {x * scalar, y * scalar, z * scalar};
    }

    template <typename ScalarType>
    friend auto operator*(ScalarType &&scalar, Vector3<T> vector)
        -> typename std::enable_if<std::is_scalar<typename std::decay<decltype(scalar)>::type>::value, Vector3<T>>::type
    {
        return {vector.x * scalar, vector.y * scalar, vector.z * scalar};
    }

    template <typename ScalarType>
    auto operator/(ScalarType &&scalar) const
        -> typename std::enable_if<std::is_scalar<typename std::decay<decltype(scalar)>::type>::value, Vector3<T>>::type
    {
        return {x / scalar, y / scalar, z / scalar};
    }

    friend Vector3<T> operator-(Vector3<T> vec)
    {
        return {-vec.x, -vec.y, -vec.z};
    }

    // comparison operators
    template <typename VectorType>
    bool operator==(VectorType &&other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }

    template <typename VectorType>
    bool operator!=(VectorType &&other) const
    {
        return x != other.x || y != other.y || z != other.z;
    }

    template <typename VectorType>
    T dot(VectorType &&other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    T sqr_length() const
    {
        return dot(*this);
    }

    T length() const
    {
        return std::sqrt(sqr_length());
    }

    Vector3<T> normalized() const
    {
        return (*this) / length();
    }
};

// Calculate standard dot product
template <typename T>
T dot(Vector3<T> a, Vector3<T> b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Calculate squared length of a vector
template <typename T>
T sqr_length(Vector3<T> vector)
{
    return dot(vector, vector);
}

// Calculate squared length of a vector
template <typename T>
T length(Vector3<T> vector)
{
    return std::sqrt(sqr_length(vector));
}

// Squared distance of 2 points
template <typename T>
T sqr_distance(Vector3<T> a, Vector3<T> b)
{
    return sqr_length(a - b);
}

template <typename T>
T distance(Vector3<T> a, Vector3<T> b)
{
    return std::sqrt(sqr_distance(a, b));
}

// normalize a floating point vector (return the argument if its close to zero)
template <typename T>
Vector3<T> normalize(Vector3<T> vector)
{
    T length = vector.length();
    if (length < std::numeric_limits<T>::epsilon())
        return vector;
    vector /= length;
    return vector;
}

// print a vector to the output stream
template <typename T>
std::string to_string(Vector3<T> vector)
{
    std::ostringstream oss;
    oss << "[" << vector.x << ", " << vector.y << ", " << vector.z << "]";
    return oss.str();
}

template <typename T>
std::ostream &operator<<(std::ostream &output, Vector3<T> vector)
{
    output << to_string(vector);
    return output;
}

#endif // VECTOR3_HPP