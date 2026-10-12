#include "render/GpuMesh.hpp"

#include <utility>

namespace {

GLuint uploadVec3Attribute(GLuint location, const std::vector<Vec3>& values)
{
	GLuint buffer = 0;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(values.size() * sizeof(Vec3)), values.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), nullptr);
	return buffer;
}

} // namespace

GpuMesh::GpuMesh(const MeshData& data) : indexCount_(static_cast<GLsizei>(data.indices.size()))
{
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);
	positionBuffer_ = uploadVec3Attribute(0, data.positions);
	normalBuffer_ = uploadVec3Attribute(1, data.normals);

	glGenBuffers(1, &indexBuffer_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(data.indices.size() * sizeof(unsigned int)),
				 data.indices.data(), GL_STATIC_DRAW);
	glBindVertexArray(0);
}

GpuMesh::~GpuMesh()
{
	release();
}

GpuMesh::GpuMesh(GpuMesh&& other) noexcept
	: vao_(std::exchange(other.vao_, 0)),
	  positionBuffer_(std::exchange(other.positionBuffer_, 0)),
	  normalBuffer_(std::exchange(other.normalBuffer_, 0)),
	  indexBuffer_(std::exchange(other.indexBuffer_, 0)),
	  indexCount_(std::exchange(other.indexCount_, 0))
{
}

GpuMesh& GpuMesh::operator=(GpuMesh&& other) noexcept
{
	if (this != &other)
	{
		release();
		vao_ = std::exchange(other.vao_, 0);
		positionBuffer_ = std::exchange(other.positionBuffer_, 0);
		normalBuffer_ = std::exchange(other.normalBuffer_, 0);
		indexBuffer_ = std::exchange(other.indexBuffer_, 0);
		indexCount_ = std::exchange(other.indexCount_, 0);
	}
	return *this;
}

void GpuMesh::draw() const
{
	glBindVertexArray(vao_);
	glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void GpuMesh::release()
{
	if (vao_ == 0)
		return;
	const GLuint buffers[] = {positionBuffer_, normalBuffer_, indexBuffer_};
	glDeleteBuffers(3, buffers);
	glDeleteVertexArrays(1, &vao_);
	vao_ = 0;
}
