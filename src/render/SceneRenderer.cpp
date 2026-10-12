#include "render/SceneRenderer.hpp"

#include "render/PartDrawers.hpp"
#include "render/PlanarShadow.hpp"

namespace {

const Vec3 LIGHT_DIRECTION = Vec3{-0.45f, -1.0f, -0.35f}.normalized();
const Vec3 BACKGROUND = {0.075f, 0.08f, 0.10f};
constexpr float SHADOW_HEIGHT = 0.002f;	// just above the floor, avoids z-fighting

} // namespace

SceneRenderer::SceneRenderer(const std::string& shaderDirectory, const MeshData& unitShape)
	: bodyShader_(shaderDirectory + "body.vert", shaderDirectory + "body.frag"),
	  shadowShader_(shaderDirectory + "body.vert", shaderDirectory + "shadow.frag"),
	  floorShader_(shaderDirectory + "floor.vert", shaderDirectory + "floor.frag"),
	  unitShape_(unitShape)
{
	glGenVertexArrays(1, &floorVao_);
}

BodyPalette& SceneRenderer::palette()
{
	return palette_;
}

RenderOptions& SceneRenderer::options()
{
	return options_;
}

void SceneRenderer::render(const Character& character, const Pose& pose, const OrbitCamera& camera,
						   int viewportWidth, int viewportHeight)
{
	if (viewportWidth <= 0 || viewportHeight <= 0)
		return;
	const float aspect = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
	const Mat4 view = camera.viewMatrix();
	const Mat4 projection = camera.projectionMatrix(aspect);

	glViewport(0, 0, viewportWidth, viewportHeight);
	glClearColor(BACKGROUND.x, BACKGROUND.y, BACKGROUND.z, 1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	if (options_.floor)
		drawFloor(view, projection, camera.position());
	if (options_.shadow)
		drawShadow(character, pose, view, projection);
	drawBody(character, pose, view, projection, camera.position());
}

void SceneRenderer::drawFloor(const Mat4& view, const Mat4& projection, const Vec3& cameraPosition) const
{
	floorShader_.use();
	floorShader_.setMat4("uView", view);
	floorShader_.setMat4("uProjection", projection);
	floorShader_.setVec3("uCameraPosition", cameraPosition);
	floorShader_.setVec3("uBackground", BACKGROUND);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(floorVao_);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
}

void SceneRenderer::drawShadow(const Character& character, const Pose& pose, const Mat4& view,
							   const Mat4& projection) const
{
	shadowShader_.use();
	shadowShader_.setMat4("uView", view);
	shadowShader_.setMat4("uProjection", projection);

	// The stencil lets each floor pixel be darkened once, even where parts overlap.
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);	// the projection flips half of the triangles

	MatrixStack stack;
	ShadowPartDrawer drawer(shadowShader_, unitShape_, planarShadowMatrix(LIGHT_DIRECTION, SHADOW_HEIGHT));
	character.draw(stack, pose, drawer);

	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glDisable(GL_STENCIL_TEST);
}

void SceneRenderer::drawBody(const Character& character, const Pose& pose, const Mat4& view,
							 const Mat4& projection, const Vec3& cameraPosition) const
{
	bodyShader_.use();
	bodyShader_.setMat4("uView", view);
	bodyShader_.setMat4("uProjection", projection);
	bodyShader_.setVec3("uLightDirection", LIGHT_DIRECTION);
	bodyShader_.setVec3("uCameraPosition", cameraPosition);

	glPolygonMode(GL_FRONT_AND_BACK, options_.wireframe ? GL_LINE : GL_FILL);
	MatrixStack stack;
	BodyPartDrawer drawer(bodyShader_, unitShape_, palette_);
	character.draw(stack, pose, drawer);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
