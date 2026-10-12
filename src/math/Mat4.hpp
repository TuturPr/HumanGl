#pragma once

#include "math/Vec3.hpp"

// 4x4 matrix stored column-major, exactly the layout OpenGL expects.
// Element (row, col) lives at m[col * 4 + row].
class Mat4
{
public:
	Mat4();	// identity

	static Mat4 identity();
	static Mat4 translation(const Vec3& offset);
	static Mat4 scaling(const Vec3& factors);
	static Mat4 rotationX(float degrees);
	static Mat4 rotationY(float degrees);
	static Mat4 rotationZ(float degrees);
	static Mat4 rotation(float degrees, const Vec3& axis);
	static Mat4 perspective(float fovYDegrees, float aspect, float near, float far);
	static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up);

	float& operator()(int row, int col) { return m[col * 4 + row]; }
	float operator()(int row, int col) const { return m[col * 4 + row]; }

	Mat4 operator*(const Mat4& other) const;
	Vec3 transformPoint(const Vec3& p) const;		// w = 1
	Vec3 transformDirection(const Vec3& d) const;	// w = 0

	const float* data() const { return m; }

private:
	float m[16];
};
