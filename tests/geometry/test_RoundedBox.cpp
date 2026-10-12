#include <gtest/gtest.h>

#include "geometry/RoundedBox.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
constexpr float EPS = 1e-4f;

MeshData defaultBox() { return makeRoundedBox(0.1f, 4); }
}

TEST(RoundedBox, PositionsStayInsideTheUnitCube)
{
	for (const Vec3& p : defaultBox().positions)
	{
		EXPECT_LE(std::abs(p.x), 0.5f + EPS);
		EXPECT_LE(std::abs(p.y), 0.5f + EPS);
		EXPECT_LE(std::abs(p.z), 0.5f + EPS);
	}
}

TEST(RoundedBox, ExtentsReachHalfOnEveryAxisSoItIsAUnitShape)
{
	const MeshData mesh = defaultBox();
	Vec3 lo{1e9f, 1e9f, 1e9f}, hi{-1e9f, -1e9f, -1e9f};
	for (const Vec3& p : mesh.positions)
	{
		lo = {std::min(lo.x, p.x), std::min(lo.y, p.y), std::min(lo.z, p.z)};
		hi = {std::max(hi.x, p.x), std::max(hi.y, p.y), std::max(hi.z, p.z)};
	}
	EXPECT_NEAR(lo.x, -0.5f, EPS);
	EXPECT_NEAR(lo.y, -0.5f, EPS);
	EXPECT_NEAR(lo.z, -0.5f, EPS);
	EXPECT_NEAR(hi.x, 0.5f, EPS);
	EXPECT_NEAR(hi.y, 0.5f, EPS);
	EXPECT_NEAR(hi.z, 0.5f, EPS);
}

TEST(RoundedBox, ShapeIsCentredOnTheOrigin)
{
	Vec3 sum;
	for (const Vec3& p : defaultBox().positions)
		sum += p;
	const float n = static_cast<float>(defaultBox().positions.size());
	EXPECT_NEAR(sum.x / n, 0.0f, 1e-3f);
	EXPECT_NEAR(sum.y / n, 0.0f, 1e-3f);
	EXPECT_NEAR(sum.z / n, 0.0f, 1e-3f);
}

TEST(RoundedBox, EveryVertexHasANormal)
{
	const MeshData mesh = defaultBox();
	EXPECT_EQ(mesh.normals.size(), mesh.positions.size());
	EXPECT_FALSE(mesh.positions.empty());
}

TEST(RoundedBox, NormalsAreUnitLength)
{
	for (const Vec3& n : defaultBox().normals)
		EXPECT_NEAR(n.length(), 1.0f, EPS);
}

TEST(RoundedBox, NormalsPointOutward)
{
	const MeshData mesh = defaultBox();
	for (std::size_t i = 0; i < mesh.positions.size(); ++i)
		EXPECT_GT(mesh.normals[i].dot(mesh.positions[i]), 0.0f) << i;
}

TEST(RoundedBox, IndexCountIsAMultipleOfThree)
{
	EXPECT_EQ(defaultBox().indices.size() % 3, 0u);
	EXPECT_FALSE(defaultBox().indices.empty());
}

TEST(RoundedBox, IndicesReferenceExistingVertices)
{
	const MeshData mesh = defaultBox();
	for (unsigned int i : mesh.indices)
		EXPECT_LT(i, mesh.positions.size());
}

TEST(RoundedBox, TrianglesAreCounterClockwiseSeenFromOutside)
{
	const MeshData mesh = defaultBox();
	for (std::size_t t = 0; t + 2 < mesh.indices.size(); t += 3)
	{
		const unsigned int a = mesh.indices[t], b = mesh.indices[t + 1], c = mesh.indices[t + 2];
		const Vec3 faceNormal = (mesh.positions[b] - mesh.positions[a]).cross(mesh.positions[c] - mesh.positions[a]);
		if (faceNormal.length() < 1e-9f)
			continue;
		const Vec3 averageNormal = mesh.normals[a] + mesh.normals[b] + mesh.normals[c];
		EXPECT_GT(faceNormal.normalized().dot(averageNormal.normalized()), 0.0f) << "triangle " << t / 3;
	}
}

TEST(RoundedBox, TrianglesFaceAwayFromTheCentre)
{
	const MeshData mesh = defaultBox();
	for (std::size_t t = 0; t + 2 < mesh.indices.size(); t += 3)
	{
		const Vec3 pa = mesh.positions[mesh.indices[t]];
		const Vec3 pb = mesh.positions[mesh.indices[t + 1]];
		const Vec3 pc = mesh.positions[mesh.indices[t + 2]];
		const Vec3 n = (pb - pa).cross(pc - pa);
		if (n.length() < 1e-9f)
			continue;
		EXPECT_GT(n.dot((pa + pb + pc) / 3.0f), 0.0f) << "triangle " << t / 3;
	}
}

TEST(RoundedBox, MoreSegmentsGiveMoreTriangles)
{
	EXPECT_GT(makeRoundedBox(0.1f, 8).indices.size(), makeRoundedBox(0.1f, 2).indices.size());
}

TEST(RoundedBox, SmallRadiusKeepsCornersNearTheCubeCorners)
{
	const MeshData mesh = makeRoundedBox(0.01f, 3);
	float best = 1e9f;
	for (const Vec3& p : mesh.positions)
		best = std::min(best, (p - Vec3{0.5f, 0.5f, 0.5f}).length());
	EXPECT_LT(best, 0.02f);
}

TEST(RoundedBox, LargeRadiusCutsTheCorners)
{
	const MeshData mesh = makeRoundedBox(0.25f, 6);
	for (const Vec3& p : mesh.positions)
		EXPECT_LT((p - Vec3{0.5f, 0.5f, 0.5f}).length(), 1.7f);
	float best = 1e9f;
	for (const Vec3& p : mesh.positions)
		best = std::min(best, (p - Vec3{0.5f, 0.5f, 0.5f}).length());
	EXPECT_GT(best, 0.1f);
}

TEST(RoundedBox, MaximumRadiusIsAccepted)
{
	EXPECT_NO_THROW(makeRoundedBox(0.5f, 4));
}

TEST(RoundedBox, ZeroRadiusThrows)
{
	EXPECT_THROW(makeRoundedBox(0.0f, 4), std::invalid_argument);
}

TEST(RoundedBox, NegativeRadiusThrows)
{
	EXPECT_THROW(makeRoundedBox(-0.1f, 4), std::invalid_argument);
}

TEST(RoundedBox, RadiusAboveHalfThrows)
{
	EXPECT_THROW(makeRoundedBox(0.6f, 4), std::invalid_argument);
}

TEST(RoundedBox, ZeroSegmentsThrows)
{
	EXPECT_THROW(makeRoundedBox(0.1f, 0), std::invalid_argument);
}

TEST(RoundedBox, NegativeSegmentsThrows)
{
	EXPECT_THROW(makeRoundedBox(0.1f, -2), std::invalid_argument);
}
