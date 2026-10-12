#pragma once

#include <cmath>

// Minimal 3D vector used by the whole project (replaces glm::vec3).
struct Vec3
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	constexpr Vec3() = default;
	constexpr Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

	constexpr Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
	constexpr Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
	constexpr Vec3 operator-() const { return {-x, -y, -z}; }
	constexpr Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
	constexpr Vec3 operator/(float s) const { return {x / s, y / s, z / s}; }
	Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
	Vec3& operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
	Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }

	constexpr float dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
	constexpr Vec3 cross(const Vec3& o) const
	{
		return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
	}
	float length() const { return std::sqrt(dot(*this)); }
	Vec3 normalized() const
	{
		const float len = length();
		return len > 0.0f ? *this / len : Vec3{};
	}
};

constexpr Vec3 operator*(float s, const Vec3& v) { return v * s; }
