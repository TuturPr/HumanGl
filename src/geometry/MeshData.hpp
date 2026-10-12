#pragma once

#include "math/Vec3.hpp"

#include <vector>

// CPU-side triangle mesh, independent of OpenGL.
struct MeshData
{
	std::vector<Vec3> positions;
	std::vector<Vec3> normals;
	std::vector<unsigned int> indices;
};
