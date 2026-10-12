#pragma once

#include "render/BodyPalette.hpp"
#include "render/GpuMesh.hpp"
#include "render/ShaderProgram.hpp"
#include "skeleton/IPartDrawer.hpp"

// Draws the lit, coloured unit shape at the given matrix.
class BodyPartDrawer : public IPartDrawer
{
public:
	BodyPartDrawer(const ShaderProgram& shader, const GpuMesh& unitShape, const BodyPalette& palette);
	void drawUnitShape(BodyPart part, const Mat4& model) override;

private:
	const ShaderProgram& shader_;
	const GpuMesh& unitShape_;
	const BodyPalette& palette_;
};

// Draws the same unit shape flattened onto the floor by a shadow matrix.
class ShadowPartDrawer : public IPartDrawer
{
public:
	ShadowPartDrawer(const ShaderProgram& shader, const GpuMesh& unitShape, const Mat4& shadowMatrix);
	void drawUnitShape(BodyPart part, const Mat4& model) override;

private:
	const ShaderProgram& shader_;
	const GpuMesh& unitShape_;
	Mat4 shadowMatrix_;
};
