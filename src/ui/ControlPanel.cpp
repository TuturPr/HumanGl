#include "ui/ControlPanel.hpp"

#include <imgui.h>

namespace {

constexpr float MIN_PART_SIZE = 0.02f;
constexpr float MAX_PART_SIZE = 1.5f;

} // namespace

ControlPanel::ControlPanel(const AnimationLibrary& animations, Animator& animator, Character& character,
						   SceneRenderer& renderer, OrbitCamera& camera)
	: animations_(animations), animator_(animator), character_(character), renderer_(renderer), camera_(camera)
{
}

void ControlPanel::draw()
{
	ImGui::SetNextWindowPos({12.0f, 12.0f}, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({330.0f, 0.0f}, ImGuiCond_FirstUseEver);
	ImGui::Begin("HumanGL");
	if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen))
		drawAnimationSection();
	if (ImGui::CollapsingHeader("Body", ImGuiTreeNodeFlags_DefaultOpen))
		drawBodySection();
	if (ImGui::CollapsingHeader("Display"))
		drawDisplaySection();
	if (ImGui::CollapsingHeader("Controls"))
		drawHelpSection();
	ImGui::End();
}

void ControlPanel::drawAnimationSection()
{
	for (std::size_t i = 0; i < animations_.size(); ++i)
	{
		const IAnimation& animation = animations_.at(i);
		const std::string label = "[" + std::to_string(i + 1) + "] " + animation.name();
		if (ImGui::RadioButton(label.c_str(), animator_.current() == &animation))
			animator_.play(animation);
	}

	float speed = animator_.speed();
	if (ImGui::SliderFloat("Speed", &speed, 0.0f, 3.0f, "%.2fx"))
		animator_.setSpeed(speed);
	float transition = animator_.transitionDuration();
	if (ImGui::SliderFloat("Blend time", &transition, 0.0f, 2.0f, "%.2f s"))
		animator_.setTransitionDuration(transition);
	bool paused = animator_.paused();
	if (ImGui::Checkbox("Pause", &paused))
		animator_.setPaused(paused);
}

void ControlPanel::drawBodySection()
{
	HumanModel& model = character_.model();
	BodyDimensions& dimensions = model.dimensions();
	BodyPalette& palette = renderer_.palette();

	if (ImGui::BeginCombo("Part", bodyPartName(selectedPart_)))
	{
		for (std::size_t i = 0; i < BODY_PART_COUNT; ++i)
		{
			const BodyPart part = static_cast<BodyPart>(i);
			if (!model.isVisible(part))
				continue;
			if (ImGui::Selectable(bodyPartName(part), part == selectedPart_))
				selectedPart_ = part;
		}
		ImGui::EndCombo();
	}
	ImGui::Checkbox("Edit left and right together", &mirrorEdits_);

	// Every edit goes to the selected part and, if asked, to its mirror.
	const BodyPart mirror = mirrorPart(selectedPart_);
	const bool alsoMirror = mirrorEdits_ && mirror != selectedPart_;

	Vec3 size = dimensions.size(selectedPart_);
	bool resized = ImGui::SliderFloat("Width", &size.x, MIN_PART_SIZE, MAX_PART_SIZE, "%.2f");
	resized |= ImGui::SliderFloat("Length", &size.y, MIN_PART_SIZE, MAX_PART_SIZE, "%.2f");
	resized |= ImGui::SliderFloat("Depth", &size.z, MIN_PART_SIZE, MAX_PART_SIZE, "%.2f");
	if (resized)
	{
		dimensions.setSize(selectedPart_, size);
		if (alsoMirror)
			dimensions.setSize(mirror, size);
	}

	Vec3 color = palette.color(selectedPart_);
	if (ImGui::ColorEdit3("Colour", &color.x))
	{
		palette.setColor(selectedPart_, color);
		if (alsoMirror)
			palette.setColor(mirror, color);
	}

	if (ImGui::Button("Reset sizes"))
		dimensions = BodyDimensions();
	ImGui::SameLine();
	if (ImGui::Button("Reset colours"))
		palette = BodyPalette();

	bool bonusParts = model.bonusPartsEnabled();
	if (ImGui::Checkbox("Bonus parts (neck, hands, feet)", &bonusParts))
	{
		model.setBonusPartsEnabled(bonusParts);
		if (!model.isVisible(selectedPart_))
			selectedPart_ = BodyPart::Torso;
	}
}

void ControlPanel::drawDisplaySection()
{
	RenderOptions& options = renderer_.options();
	ImGui::Checkbox("Wireframe", &options.wireframe);
	ImGui::Checkbox("Shadow", &options.shadow);
	ImGui::Checkbox("Floor", &options.floor);
	if (ImGui::Button("Reset camera"))
		camera_ = OrbitCamera();
}

void ControlPanel::drawHelpSection()
{
	ImGui::BulletText("Left drag: turn around the character");
	ImGui::BulletText("Mouse wheel: zoom");
	ImGui::BulletText("1 - %zu: choose an animation", animations_.size());
	ImGui::BulletText("Space: pause / resume");
	ImGui::BulletText("Escape: quit");
}
