#include <gtest/gtest.h>

#include "render/PlanarShadow.hpp"

namespace {
constexpr float EPS = 1e-4f;

const Vec3 LIGHT{0.3f, -1.0f, 0.2f};
const Vec3 POINTS[] = {{0, 1, 0}, {1, 2, -1}, {-0.5f, 0.3f, 2}, {0.2f, 5, 0.7f}};

void expectOnRay(const Vec3& original, const Vec3& projected, const Vec3& light)
{
	EXPECT_NEAR((projected - original).cross(light).length(), 0.0f, EPS);
}
}

TEST(PlanarShadow, ProjectedPointsLieOnTheGroundPlane)
{
	const Mat4 m = planarShadowMatrix(LIGHT, 0.0f);
	for (const Vec3& p : POINTS)
		EXPECT_NEAR(m.transformPoint(p).y, 0.0f, EPS);
}

TEST(PlanarShadow, ProjectedPointsLieAtTheRequestedHeight)
{
	const Mat4 m = planarShadowMatrix(LIGHT, 0.01f);
	for (const Vec3& p : POINTS)
		EXPECT_NEAR(m.transformPoint(p).y, 0.01f, EPS);
}

TEST(PlanarShadow, ProjectedPointsLieOnTheLightRayThroughTheOriginalPoint)
{
	for (float height : {0.0f, 0.02f, -0.5f})
	{
		const Mat4 m = planarShadowMatrix(LIGHT, height);
		for (const Vec3& p : POINTS)
			expectOnRay(p, m.transformPoint(p), LIGHT);
	}
}

TEST(PlanarShadow, ShadowIsDisplacedAlongTheLightDirection)
{
	const Mat4 m = planarShadowMatrix({1.0f, -1.0f, 0.0f}, 0.0f);
	const Vec3 s = m.transformPoint({0.0f, 2.0f, 0.0f});
	EXPECT_NEAR(s.x, 2.0f, EPS);
	EXPECT_NEAR(s.z, 0.0f, EPS);
}

TEST(PlanarShadow, VerticalLightCastsShadowStraightDown)
{
	const Mat4 m = planarShadowMatrix({0.0f, -1.0f, 0.0f}, 0.0f);
	const Vec3 s = m.transformPoint({0.4f, 3.0f, -0.6f});
	EXPECT_NEAR(s.x, 0.4f, EPS);
	EXPECT_NEAR(s.y, 0.0f, EPS);
	EXPECT_NEAR(s.z, -0.6f, EPS);
}

TEST(PlanarShadow, PointsAlreadyOnThePlaneDoNotMove)
{
	const Mat4 m = planarShadowMatrix(LIGHT, 0.0f);
	const Vec3 s = m.transformPoint({1.5f, 0.0f, -2.0f});
	EXPECT_NEAR(s.x, 1.5f, EPS);
	EXPECT_NEAR(s.y, 0.0f, EPS);
	EXPECT_NEAR(s.z, -2.0f, EPS);
}

TEST(PlanarShadow, ShadowIsFlatEvenForPointsAtDifferentHeights)
{
	const Mat4 m = planarShadowMatrix(LIGHT, 0.0f);
	EXPECT_NEAR(m.transformPoint({0, 0.1f, 0}).y, m.transformPoint({0, 9.0f, 0}).y, EPS);
}

TEST(PlanarShadow, HigherPointsCastShadowsFurtherFromTheirFoot)
{
	const Mat4 m = planarShadowMatrix(LIGHT, 0.0f);
	const Vec3 low = m.transformPoint({0, 1.0f, 0});
	const Vec3 high = m.transformPoint({0, 2.0f, 0});
	EXPECT_GT(std::abs(high.x), std::abs(low.x));
}
