#include "render/PlanarShadow.hpp"

// p' = p - L * (p.y - h) / L.y
Mat4 planarShadowMatrix(const Vec3& lightDirection, float height)
{
	const Vec3& l = lightDirection;
	Mat4 m;
	m(0, 1) = -l.x / l.y;
	m(0, 3) = l.x * height / l.y;
	m(1, 1) = 0.0f;
	m(1, 3) = height;
	m(2, 1) = -l.z / l.y;
	m(2, 3) = l.z * height / l.y;
	return m;
}
