#include <gtest/gtest.h>

#include <stdexcept>

#include "math/MatrixStack.hpp"

namespace {
constexpr float EPS = 1e-5f;

void expectVec(const Vec3& a, const Vec3& b)
{
	EXPECT_NEAR(a.x, b.x, EPS);
	EXPECT_NEAR(a.y, b.y, EPS);
	EXPECT_NEAR(a.z, b.z, EPS);
}

void expectMat(const Mat4& a, const Mat4& b)
{
	for (int i = 0; i < 16; ++i)
		EXPECT_NEAR(a.data()[i], b.data()[i], EPS) << "element index " << i;
}
}

TEST(MatrixStack, StartsWithIdentityAndDepthOne)
{
	MatrixStack s;
	expectMat(s.top(), Mat4::identity());
	EXPECT_EQ(s.depth(), 1u);
}

TEST(MatrixStack, PushDuplicatesTop)
{
	MatrixStack s;
	s.translate(Vec3(1, 2, 3));
	s.push();
	expectMat(s.top(), Mat4::translation(Vec3(1, 2, 3)));
}

TEST(MatrixStack, PopRestoresPreviousMatrix)
{
	MatrixStack s;
	s.translate(Vec3(1, 2, 3));
	Mat4 before = s.top();
	s.push();
	s.rotateY(45.0f);
	s.scale(Vec3(2, 2, 2));
	s.pop();
	expectMat(s.top(), before);
}

TEST(MatrixStack, PopOnBaseThrowsLogicError)
{
	MatrixStack s;
	EXPECT_THROW(s.pop(), std::logic_error);
}

TEST(MatrixStack, PopMoreThanPushedThrows)
{
	MatrixStack s;
	s.push();
	s.pop();
	EXPECT_THROW(s.pop(), std::logic_error);
}

TEST(MatrixStack, DepthTracksPushAndPop)
{
	MatrixStack s;
	s.push();
	EXPECT_EQ(s.depth(), 2u);
	s.push();
	EXPECT_EQ(s.depth(), 3u);
	s.pop();
	EXPECT_EQ(s.depth(), 2u);
	s.pop();
	EXPECT_EQ(s.depth(), 1u);
}

TEST(MatrixStack, TranslateThenRotateIsPostMultiplied)
{
	MatrixStack s;
	s.translate(Vec3(10, 0, 0));
	s.rotateZ(90.0f);
	expectMat(s.top(), Mat4::translation(Vec3(10, 0, 0)) * Mat4::rotationZ(90.0f));
	expectVec(s.top().transformPoint(Vec3(1, 0, 0)), Vec3(10, 1, 0));
}

TEST(MatrixStack, RotateThenTranslateIsPostMultiplied)
{
	MatrixStack s;
	s.rotateZ(90.0f);
	s.translate(Vec3(10, 0, 0));
	expectVec(s.top().transformPoint(Vec3(0, 0, 0)), Vec3(0, 10, 0));
}

TEST(MatrixStack, PostMultiplicationOntoExistingTop)
{
	MatrixStack s;
	s.scale(Vec3(2, 2, 2));
	Mat4 base = s.top();
	s.translate(Vec3(1, 0, 0));
	s.rotateX(30.0f);
	expectMat(s.top(), base * Mat4::translation(Vec3(1, 0, 0)) * Mat4::rotationX(30.0f));
}

TEST(MatrixStack, RotateXYZMatchMat4Factories)
{
	MatrixStack s;
	s.rotateX(10.0f);
	expectMat(s.top(), Mat4::rotationX(10.0f));
	s.loadIdentity();
	s.rotateY(20.0f);
	expectMat(s.top(), Mat4::rotationY(20.0f));
	s.loadIdentity();
	s.rotateZ(30.0f);
	expectMat(s.top(), Mat4::rotationZ(30.0f));
}

TEST(MatrixStack, RotateArbitraryAxisMatchesMat4)
{
	MatrixStack s;
	s.rotate(50.0f, Vec3(1, 2, 3));
	expectMat(s.top(), Mat4::rotation(50.0f, Vec3(1, 2, 3)));
}

TEST(MatrixStack, ScaleMatchesMat4Scaling)
{
	MatrixStack s;
	s.scale(Vec3(1, 2, 3));
	expectMat(s.top(), Mat4::scaling(Vec3(1, 2, 3)));
}

TEST(MatrixStack, MultiplyPostMultipliesGivenMatrix)
{
	MatrixStack s;
	s.translate(Vec3(1, 1, 1));
	Mat4 m = Mat4::rotationY(25.0f);
	s.multiply(m);
	expectMat(s.top(), Mat4::translation(Vec3(1, 1, 1)) * m);
}

TEST(MatrixStack, LoadIdentityResetsTopOnly)
{
	MatrixStack s;
	s.translate(Vec3(1, 2, 3));
	s.push();
	s.loadIdentity();
	expectMat(s.top(), Mat4::identity());
	s.pop();
	expectMat(s.top(), Mat4::translation(Vec3(1, 2, 3)));
}

TEST(MatrixStack, ParentRotationAffectsChildPosition)
{
	MatrixStack s;
	s.rotateZ(90.0f);		// parent (torso)
	s.push();
	s.translate(Vec3(2, 0, 0));	// child offset along parent's X
	expectVec(s.top().transformPoint(Vec3(0, 0, 0)), Vec3(0, 2, 0));
	s.pop();
}

TEST(MatrixStack, ChildTransformDoesNotAffectParentAfterPop)
{
	MatrixStack s;
	s.translate(Vec3(1, 0, 0));
	Mat4 parent = s.top();
	s.push();
	s.rotateZ(90.0f);
	s.translate(Vec3(0, 5, 0));
	s.pop();
	expectMat(s.top(), parent);
	expectVec(s.top().transformPoint(Vec3(0, 0, 0)), Vec3(1, 0, 0));
}

TEST(MatrixStack, SiblingsStartFromSameParentTransform)
{
	MatrixStack s;
	s.translate(Vec3(0, 1, 0));
	s.push();
	s.translate(Vec3(1, 0, 0));
	Vec3 a = s.top().transformPoint(Vec3());
	s.pop();
	s.push();
	s.translate(Vec3(-1, 0, 0));
	Vec3 b = s.top().transformPoint(Vec3());
	s.pop();
	expectVec(a, Vec3(1, 1, 0));
	expectVec(b, Vec3(-1, 1, 0));
}

TEST(MatrixStack, TwoLevelHierarchyGrandchildComposesBothParents)
{
	MatrixStack s;
	s.rotateZ(90.0f);			// torso
	s.push();
	s.translate(Vec3(2, 0, 0));		// upper arm
	s.rotateZ(90.0f);
	s.push();
	s.translate(Vec3(1, 0, 0));		// forearm
	// Arm points along +Y (torso) then rotates to -X; forearm origin at (0,2,0)+(-1,0,0).
	expectVec(s.top().transformPoint(Vec3()), Vec3(-1, 2, 0));
	s.pop();
	s.pop();
	EXPECT_EQ(s.depth(), 1u);
}
