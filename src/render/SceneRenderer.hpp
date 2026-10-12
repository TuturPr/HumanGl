#pragma once

#include "render/BodyPalette.hpp"
#include "render/GpuMesh.hpp"
#include "render/OrbitCamera.hpp"
#include "render/ShaderProgram.hpp"
#include "skeleton/Character.hpp"

#include <string>

struct RenderOptions
{
	bool wireframe = false;
	bool shadow = true;
	bool floor = true;
};

// Draws one frame: floor, the character's planar shadow, then the character.
class SceneRenderer
{
public:
	SceneRenderer(const std::string& shaderDirectory, const MeshData& unitShape);

	BodyPalette& palette();
	RenderOptions& options();

	void render(const Character& character, const Pose& pose, const OrbitCamera& camera,
				int viewportWidth, int viewportHeight);

private:
	void drawFloor(const Mat4& view, const Mat4& projection, const Vec3& cameraPosition) const;
	void drawShadow(const Character& character, const Pose& pose, const Mat4& view, const Mat4& projection) const;
	void drawBody(const Character& character, const Pose& pose, const Mat4& view, const Mat4& projection,
				  const Vec3& cameraPosition) const;

	ShaderProgram bodyShader_;
	ShaderProgram shadowShader_;
	ShaderProgram floorShader_;
	GpuMesh unitShape_;
	GLuint floorVao_ = 0;	// the floor quad is generated in its vertex shader
	BodyPalette palette_;
	RenderOptions options_;
};
