#pragma once

#include "geometry/MeshData.hpp"

#include <glad/glad.h>

// A MeshData uploaded to the GPU (VAO + VBOs + EBO). Owns its GL objects:
// non-copyable, movable.
class GpuMesh
{
public:
	explicit GpuMesh(const MeshData& data);
	~GpuMesh();

	GpuMesh(const GpuMesh&) = delete;
	GpuMesh& operator=(const GpuMesh&) = delete;
	GpuMesh(GpuMesh&& other) noexcept;
	GpuMesh& operator=(GpuMesh&& other) noexcept;

	void draw() const;

private:
	void release();

	GLuint vao_ = 0;
	GLuint positionBuffer_ = 0;
	GLuint normalBuffer_ = 0;
	GLuint indexBuffer_ = 0;
	GLsizei indexCount_ = 0;
};
