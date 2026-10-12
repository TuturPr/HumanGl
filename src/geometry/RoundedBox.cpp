#include "geometry/RoundedBox.hpp"

#include "math/MathUtils.hpp"

#include <array>
#include <cmath>
#include <stdexcept>

namespace {

// Grid coordinates along one axis of a face, from -0.5 to 0.5. Samples are
// spaced by equal angles inside both rounded bands; the flat middle of the
// face needs no intermediate vertex.
std::vector<float> faceCoordinates(float radius, int segments)
{
	std::vector<float> coords;
	const float inner = 0.5f - radius;
	for (int i = 0; i <= segments; ++i)
	{
		const float angle = (math::PI / 2.0f) * static_cast<float>(i) / static_cast<float>(segments);
		coords.push_back(-inner - radius * std::cos(angle));
	}
	for (int i = segments; i >= 0; --i)
	{
		const float angle = (math::PI / 2.0f) * static_cast<float>(i) / static_cast<float>(segments);
		coords.push_back(inner + radius * std::cos(angle));
	}
	return coords;
}

struct Face
{
	Vec3 normal;
	Vec3 u;
	Vec3 v;	// u x v == normal, so triangles are counter-clockwise from outside
};

} // namespace

MeshData makeRoundedBox(float radius, int segments)
{
	if (radius <= 0.0f || radius > 0.5f || segments < 1)
		throw std::invalid_argument("makeRoundedBox: invalid radius or segment count");

	const std::array<Face, 6> faces = {{
		{{1, 0, 0}, {0, 0, -1}, {0, 1, 0}},
		{{-1, 0, 0}, {0, 0, 1}, {0, 1, 0}},
		{{0, 1, 0}, {1, 0, 0}, {0, 0, -1}},
		{{0, -1, 0}, {1, 0, 0}, {0, 0, 1}},
		{{0, 0, 1}, {1, 0, 0}, {0, 1, 0}},
		{{0, 0, -1}, {-1, 0, 0}, {0, 1, 0}},
	}};
	const std::vector<float> coords = faceCoordinates(radius, segments);
	const int n = static_cast<int>(coords.size());
	const float inner = 0.5f - radius;

	MeshData mesh;
	for (const Face& face : faces)
	{
		const unsigned int first = static_cast<unsigned int>(mesh.positions.size());
		for (int j = 0; j < n; ++j)
		{
			for (int i = 0; i < n; ++i)
			{
				// Point on the sharp cube, pushed onto the rounded surface: the
				// closest point of the inner box plus `radius` along the normal.
				const Vec3 onCube = face.normal * 0.5f + face.u * coords[i] + face.v * coords[j];
				const Vec3 core = {
					math::clamp(onCube.x, -inner, inner),
					math::clamp(onCube.y, -inner, inner),
					math::clamp(onCube.z, -inner, inner),
				};
				const Vec3 normal = (onCube - core).normalized();
				mesh.positions.push_back(core + normal * radius);
				mesh.normals.push_back(normal);
			}
		}
		for (int j = 0; j + 1 < n; ++j)
		{
			for (int i = 0; i + 1 < n; ++i)
			{
				const unsigned int a = first + static_cast<unsigned int>(j * n + i);
				const unsigned int b = a + 1;
				const unsigned int c = a + static_cast<unsigned int>(n);
				const unsigned int d = c + 1;
				mesh.indices.insert(mesh.indices.end(), {a, b, d, a, d, c});
			}
		}
	}
	return mesh;
}
