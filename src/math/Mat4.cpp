#include "math/Mat4.hpp"
#include "math/MathUtils.hpp"

#include <cmath>

Mat4::Mat4() : m{}
{
	for (int i = 0; i < 4; ++i)
		(*this)(i, i) = 1.0f;
}

Mat4 Mat4::identity()
{
	return Mat4();
}

Mat4 Mat4::translation(const Vec3& offset)
{
	Mat4 r;
	r(0, 3) = offset.x;
	r(1, 3) = offset.y;
	r(2, 3) = offset.z;
	return r;
}

Mat4 Mat4::scaling(const Vec3& factors)
{
	Mat4 r;
	r(0, 0) = factors.x;
	r(1, 1) = factors.y;
	r(2, 2) = factors.z;
	return r;
}

Mat4 Mat4::rotationX(float degrees)
{
	const float c = std::cos(math::radians(degrees));
	const float s = std::sin(math::radians(degrees));
	Mat4 r;
	r(1, 1) = c;  r(1, 2) = -s;
	r(2, 1) = s;  r(2, 2) = c;
	return r;
}

Mat4 Mat4::rotationY(float degrees)
{
	const float c = std::cos(math::radians(degrees));
	const float s = std::sin(math::radians(degrees));
	Mat4 r;
	r(0, 0) = c;  r(0, 2) = s;
	r(2, 0) = -s; r(2, 2) = c;
	return r;
}

Mat4 Mat4::rotationZ(float degrees)
{
	const float c = std::cos(math::radians(degrees));
	const float s = std::sin(math::radians(degrees));
	Mat4 r;
	r(0, 0) = c;  r(0, 1) = -s;
	r(1, 0) = s;  r(1, 1) = c;
	return r;
}

// Rodrigues' rotation formula around an arbitrary (normalized) axis.
Mat4 Mat4::rotation(float degrees, const Vec3& axis)
{
	const Vec3 a = axis.normalized();
	const float c = std::cos(math::radians(degrees));
	const float s = std::sin(math::radians(degrees));
	const float t = 1.0f - c;
	Mat4 r;
	r(0, 0) = t * a.x * a.x + c;       r(0, 1) = t * a.x * a.y - s * a.z; r(0, 2) = t * a.x * a.z + s * a.y;
	r(1, 0) = t * a.x * a.y + s * a.z; r(1, 1) = t * a.y * a.y + c;       r(1, 2) = t * a.y * a.z - s * a.x;
	r(2, 0) = t * a.x * a.z - s * a.y; r(2, 1) = t * a.y * a.z + s * a.x; r(2, 2) = t * a.z * a.z + c;
	return r;
}

Mat4 Mat4::perspective(float fovYDegrees, float aspect, float near, float far)
{
	const float f = 1.0f / std::tan(math::radians(fovYDegrees) / 2.0f);
	Mat4 r;
	r(0, 0) = f / aspect;
	r(1, 1) = f;
	r(2, 2) = (far + near) / (near - far);
	r(2, 3) = (2.0f * far * near) / (near - far);
	r(3, 2) = -1.0f;
	r(3, 3) = 0.0f;
	return r;
}

Mat4 Mat4::lookAt(const Vec3& eye, const Vec3& target, const Vec3& up)
{
	const Vec3 f = (target - eye).normalized();
	const Vec3 s = f.cross(up).normalized();
	const Vec3 u = s.cross(f);
	Mat4 r;
	r(0, 0) = s.x;  r(0, 1) = s.y;  r(0, 2) = s.z;  r(0, 3) = -s.dot(eye);
	r(1, 0) = u.x;  r(1, 1) = u.y;  r(1, 2) = u.z;  r(1, 3) = -u.dot(eye);
	r(2, 0) = -f.x; r(2, 1) = -f.y; r(2, 2) = -f.z; r(2, 3) = f.dot(eye);
	return r;
}

Mat4 Mat4::operator*(const Mat4& other) const
{
	Mat4 r;
	for (int row = 0; row < 4; ++row)
	{
		for (int col = 0; col < 4; ++col)
		{
			float sum = 0.0f;
			for (int k = 0; k < 4; ++k)
				sum += (*this)(row, k) * other(k, col);
			r(row, col) = sum;
		}
	}
	return r;
}

Vec3 Mat4::transformPoint(const Vec3& p) const
{
	const Mat4& a = *this;
	return {
		a(0, 0) * p.x + a(0, 1) * p.y + a(0, 2) * p.z + a(0, 3),
		a(1, 0) * p.x + a(1, 1) * p.y + a(1, 2) * p.z + a(1, 3),
		a(2, 0) * p.x + a(2, 1) * p.y + a(2, 2) * p.z + a(2, 3),
	};
}

Vec3 Mat4::transformDirection(const Vec3& d) const
{
	const Mat4& a = *this;
	return {
		a(0, 0) * d.x + a(0, 1) * d.y + a(0, 2) * d.z,
		a(1, 0) * d.x + a(1, 1) * d.y + a(1, 2) * d.z,
		a(2, 0) * d.x + a(2, 1) * d.y + a(2, 2) * d.z,
	};
}
