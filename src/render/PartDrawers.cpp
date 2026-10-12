#include "render/PartDrawers.hpp"

BodyPartDrawer::BodyPartDrawer(const ShaderProgram& shader, const GpuMesh& unitShape, const BodyPalette& palette)
	: shader_(shader), unitShape_(unitShape), palette_(palette)
{
}

void BodyPartDrawer::drawUnitShape(BodyPart part, const Mat4& model)
{
	shader_.setMat4("uModel", model);
	shader_.setVec3("uColor", palette_.color(part));
	unitShape_.draw();
}

ShadowPartDrawer::ShadowPartDrawer(const ShaderProgram& shader, const GpuMesh& unitShape, const Mat4& shadowMatrix)
	: shader_(shader), unitShape_(unitShape), shadowMatrix_(shadowMatrix)
{
}

void ShadowPartDrawer::drawUnitShape(BodyPart, const Mat4& model)
{
	shader_.setMat4("uModel", shadowMatrix_ * model);
	unitShape_.draw();
}
