#include <gtest/gtest.h>

#include "math/Mat4.hpp"
#include "math/MathUtils.hpp"

namespace {
constexpr float EPS = 1e-5f;

void expectVec(const Vec3& a, const Vec3& b, float eps = EPS)
{
	EXPECT_NEAR(a.x, b.x, eps);
	EXPECT_NEAR(a.y, b.y, eps);
	EXPECT_NEAR(a.z, b.z, eps);
}

void expectMat(const Mat4& a, const Mat4& b, float eps = EPS)
{
	for (int i = 0; i < 16; ++i)
		EXPECT_NEAR(a.data()[i], b.data()[i], eps) << "element index " << i;
}
}

TEST(Mat4, DefaultConstructorIsIdentity)
{
	Mat4 m;
	for (int r = 0; r < 4; ++r)
		for (int c = 0; c < 4; ++c)
			EXPECT_FLOAT_EQ(m(r, c), r == c ? 1.0f : 0.0f);
}

TEST(Mat4, IdentityFactoryMatchesDefault) { expectMat(Mat4::identity(), Mat4()); }

TEST(Mat4, IdentityLeavesPointUnchanged)
{
	expectVec(Mat4::identity().transformPoint(Vec3(1, -2, 3)), Vec3(1, -2, 3));
}

TEST(Mat4, MultiplyByIdentityIsNoOp)
{
	Mat4 m = Mat4::translation(Vec3(1, 2, 3)) * Mat4::rotationY(30.0f);
	expectMat(m * Mat4::identity(), m);
	expectMat(Mat4::identity() * m, m);
}

TEST(Mat4, ElementAccessorUsesColumnMajorStorage)
{
	Mat4 m;
	m(1, 2) = 7.0f;
	EXPECT_FLOAT_EQ(m.data()[2 * 4 + 1], 7.0f);
}

TEST(Mat4, TranslationIsStoredInDataElements12To14)
{
	Mat4 t = Mat4::translation(Vec3(1, 2, 3));
	EXPECT_FLOAT_EQ(t.data()[12], 1.0f);
	EXPECT_FLOAT_EQ(t.data()[13], 2.0f);
	EXPECT_FLOAT_EQ(t.data()[14], 3.0f);
	EXPECT_FLOAT_EQ(t.data()[15], 1.0f);
}

TEST(Mat4, TranslationMovesPoint)
{
	expectVec(Mat4::translation(Vec3(1, 2, 3)).transformPoint(Vec3(1, 1, 1)), Vec3(2, 3, 4));
}

TEST(Mat4, TransformDirectionIgnoresTranslation)
{
	expectVec(Mat4::translation(Vec3(5, 6, 7)).transformDirection(Vec3(1, 0, 0)), Vec3(1, 0, 0));
}

TEST(Mat4, TransformDirectionAppliesRotation)
{
	Mat4 m = Mat4::translation(Vec3(5, 6, 7)) * Mat4::rotationZ(90.0f);
	expectVec(m.transformDirection(Vec3(1, 0, 0)), Vec3(0, 1, 0));
}

TEST(Mat4, ScalingScalesEachAxis)
{
	expectVec(Mat4::scaling(Vec3(2, 3, 4)).transformPoint(Vec3(1, 1, 1)), Vec3(2, 3, 4));
}

TEST(Mat4, ScalingDiagonalLayout)
{
	Mat4 s = Mat4::scaling(Vec3(2, 3, 4));
	EXPECT_FLOAT_EQ(s.data()[0], 2.0f);
	EXPECT_FLOAT_EQ(s.data()[5], 3.0f);
	EXPECT_FLOAT_EQ(s.data()[10], 4.0f);
}

TEST(Mat4, RotationZ90MapsXToY)
{
	expectVec(Mat4::rotationZ(90.0f).transformPoint(Vec3(1, 0, 0)), Vec3(0, 1, 0));
}

TEST(Mat4, RotationX90MapsYToZ)
{
	expectVec(Mat4::rotationX(90.0f).transformPoint(Vec3(0, 1, 0)), Vec3(0, 0, 1));
}

TEST(Mat4, RotationY90MapsZToX)
{
	expectVec(Mat4::rotationY(90.0f).transformPoint(Vec3(0, 0, 1)), Vec3(1, 0, 0));
}

TEST(Mat4, RotationZ180FlipsXAndY)
{
	expectVec(Mat4::rotationZ(180.0f).transformPoint(Vec3(1, 2, 3)), Vec3(-1, -2, 3));
}

TEST(Mat4, RotationByZeroIsIdentity) { expectMat(Mat4::rotationX(0.0f), Mat4::identity()); }

TEST(Mat4, NegativeAngleUndoesPositiveAngle)
{
	expectMat(Mat4::rotationY(40.0f) * Mat4::rotationY(-40.0f), Mat4::identity());
}

TEST(Mat4, ArbitraryAxisXMatchesRotationX)
{
	expectMat(Mat4::rotation(33.0f, Vec3(1, 0, 0)), Mat4::rotationX(33.0f));
}

TEST(Mat4, ArbitraryAxisYMatchesRotationY)
{
	expectMat(Mat4::rotation(33.0f, Vec3(0, 1, 0)), Mat4::rotationY(33.0f));
}

TEST(Mat4, ArbitraryAxisZMatchesRotationZ)
{
	expectMat(Mat4::rotation(33.0f, Vec3(0, 0, 1)), Mat4::rotationZ(33.0f));
}

TEST(Mat4, ArbitraryAxisIsNormalizedInternally)
{
	expectMat(Mat4::rotation(33.0f, Vec3(0, 0, 5)), Mat4::rotationZ(33.0f));
}

TEST(Mat4, ArbitraryAxisKeepsAxisFixed)
{
	Vec3 axis = Vec3(1, 1, 1).normalized();
	expectVec(Mat4::rotation(77.0f, axis).transformPoint(axis), axis);
}

TEST(Mat4, RotationAroundDiagonalCyclesAxes)
{
	// 120 degrees around (1,1,1) maps X -> Y.
	expectVec(Mat4::rotation(120.0f, Vec3(1, 1, 1)).transformPoint(Vec3(1, 0, 0)), Vec3(0, 1, 0));
}

TEST(Mat4, RotationPreservesLength)
{
	Vec3 v(1.0f, -2.0f, 3.0f);
	Vec3 r = Mat4::rotation(57.0f, Vec3(1, 2, 3)).transformPoint(v);
	EXPECT_NEAR(r.length(), v.length(), 1e-4f);
}

TEST(Mat4, CompositionOrderTranslationTimesRotationRotatesThenTranslates)
{
	Mat4 m = Mat4::translation(Vec3(10, 0, 0)) * Mat4::rotationZ(90.0f);
	expectVec(m.transformPoint(Vec3(1, 0, 0)), Vec3(10, 1, 0));
}

TEST(Mat4, CompositionOrderRotationTimesTranslationTranslatesThenRotates)
{
	Mat4 m = Mat4::rotationZ(90.0f) * Mat4::translation(Vec3(10, 0, 0));
	expectVec(m.transformPoint(Vec3(1, 0, 0)), Vec3(0, 11, 0));
}

TEST(Mat4, MultiplicationIsAssociative)
{
	Mat4 a = Mat4::rotationX(20.0f), b = Mat4::translation(Vec3(1, 2, 3)), c = Mat4::scaling(Vec3(2, 2, 2));
	expectMat((a * b) * c, a * (b * c));
}

TEST(Mat4, PerspectiveMapsNearPlaneToMinusOne)
{
	Mat4 p = Mat4::perspective(60.0f, 1.5f, 0.1f, 100.0f);
	Vec3 clip = p.transformPoint(Vec3(0, 0, -0.1f));
	const float* d = p.data();
	float w = d[11] * -0.1f + d[15];
	EXPECT_NEAR(clip.z / w, -1.0f, 1e-4f);
}

TEST(Mat4, PerspectiveMapsFarPlaneToPlusOne)
{
	Mat4 p = Mat4::perspective(60.0f, 1.5f, 0.1f, 100.0f);
	Vec3 clip = p.transformPoint(Vec3(0, 0, -100.0f));
	float w = p.data()[11] * -100.0f + p.data()[15];
	EXPECT_NEAR(clip.z / w, 1.0f, 1e-4f);
}

TEST(Mat4, PerspectiveWEqualsMinusZ)
{
	Mat4 p = Mat4::perspective(45.0f, 1.0f, 0.1f, 50.0f);
	EXPECT_FLOAT_EQ(p(3, 2), -1.0f);
	EXPECT_FLOAT_EQ(p(3, 3), 0.0f);
}

TEST(Mat4, PerspectiveHalfFovEdgeMapsToNdcOne)
{
	// With fovY = 90 deg, a point at (0, d, -d) lies on the top edge of the frustum.
	Mat4 p = Mat4::perspective(90.0f, 1.0f, 0.1f, 100.0f);
	Vec3 clip = p.transformPoint(Vec3(0, 5, -5));
	EXPECT_NEAR(clip.y / 5.0f, 1.0f, 1e-4f);
}

TEST(Mat4, PerspectiveAspectShrinksX)
{
	Mat4 p = Mat4::perspective(90.0f, 2.0f, 0.1f, 100.0f);
	EXPECT_NEAR(p(0, 0), 0.5f, EPS);
	EXPECT_NEAR(p(1, 1), 1.0f, EPS);
}

TEST(Mat4, LookAtMapsEyeToOrigin)
{
	Mat4 v = Mat4::lookAt(Vec3(3, 4, 5), Vec3(0, 0, 0), Vec3(0, 1, 0));
	expectVec(v.transformPoint(Vec3(3, 4, 5)), Vec3(0, 0, 0), 1e-4f);
}

TEST(Mat4, LookAtMapsTargetOntoNegativeZ)
{
	Vec3 eye(3, 4, 5), target(-1, 2, 0);
	Mat4 v = Mat4::lookAt(eye, target, Vec3(0, 1, 0));
	Vec3 p = v.transformPoint(target);
	EXPECT_NEAR(p.x, 0.0f, 1e-4f);
	EXPECT_NEAR(p.y, 0.0f, 1e-4f);
	EXPECT_NEAR(p.z, -(target - eye).length(), 1e-4f);
}

TEST(Mat4, LookAtFromPositiveZIsPureTranslation)
{
	Mat4 v = Mat4::lookAt(Vec3(0, 0, 5), Vec3(0, 0, 0), Vec3(0, 1, 0));
	expectMat(v, Mat4::translation(Vec3(0, 0, -5)));
}

TEST(Mat4, LookAtKeepsUpDirectionPointingUp)
{
	Mat4 v = Mat4::lookAt(Vec3(0, 0, 5), Vec3(0, 0, 0), Vec3(0, 1, 0));
	expectVec(v.transformDirection(Vec3(0, 1, 0)), Vec3(0, 1, 0));
}

TEST(Mat4, LookAtFromPositiveXPutsOriginOnNegativeZ)
{
	Mat4 v = Mat4::lookAt(Vec3(5, 0, 0), Vec3(0, 0, 0), Vec3(0, 1, 0));
	expectVec(v.transformPoint(Vec3(0, 0, 0)), Vec3(0, 0, -5));
	// Looking down -X with +Y up, the camera right vector is world -Z.
	expectVec(v.transformDirection(Vec3(0, 0, -1)), Vec3(1, 0, 0));
}

TEST(Mat4, RotationAroundZeroAxisIsIdentity)
{
	const Mat4 r = Mat4::rotation(45.0f, Vec3(0, 0, 0));
	for (int row = 0; row < 4; ++row)
		for (int col = 0; col < 4; ++col)
			EXPECT_NEAR(r(row, col), row == col ? 1.0f : 0.0f, 1e-6f);
}
