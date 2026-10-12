#include "animation/clips/Clips.hpp"

#include "math/MathUtils.hpp"

#include <cmath>

namespace clips {
namespace {

// Standing still: slow breathing and a light weight shift. Built from sine
// waves, which loop seamlessly and never stop moving abruptly.
class IdleAnimation : public IAnimation
{
public:
	const std::string& name() const override { return name_; }

	Pose sample(float time) const override
	{
		const float breath = std::sin(2.0f * math::PI * time / BREATH_PERIOD);
		const float sway = std::sin(2.0f * math::PI * time / SWAY_PERIOD);

		Pose pose;
		pose.rootRoll = 1.5f * sway;
		pose.neckPitch = 3.0f + 2.0f * breath;
		pose.neckRoll = -1.0f * sway;

		for (ArmPose* arm : {&pose.leftArm, &pose.rightArm})
		{
			arm->shoulderAbduct = 6.0f + 1.5f * breath;
			arm->shoulderFlex = 4.0f;
			arm->elbowFlex = 12.0f + 3.0f * breath;
			arm->wristFlex = 8.0f;
		}
		// Weight shifts onto one leg: that hip opens a little, the other knee softens.
		pose.leftLeg  = {2.0f, 2.0f + 1.5f * sway, 3.0f + 3.0f * std::max(0.0f, -sway), 0.0f};
		pose.rightLeg = {2.0f, 2.0f - 1.5f * sway, 3.0f + 3.0f * std::max(0.0f, sway), 0.0f};
		pose.leftLeg.ankleFlex = pose.leftLeg.kneeFlex / 2.0f;
		pose.rightLeg.ankleFlex = pose.rightLeg.kneeFlex / 2.0f;
		return pose;
	}

private:
	static constexpr float BREATH_PERIOD = 3.5f;
	static constexpr float SWAY_PERIOD = 7.0f;
	std::string name_ = "Idle";
};

} // namespace

std::unique_ptr<IAnimation> createIdle()
{
	return std::make_unique<IdleAnimation>();
}

} // namespace clips
