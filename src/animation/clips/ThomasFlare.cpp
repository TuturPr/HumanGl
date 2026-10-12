#include "animation/clips/Clips.hpp"

#include "math/MathUtils.hpp"

#include <cmath>

namespace clips {
namespace {

// Thomas flare, the breakdance power move.
//
// The body lies face down, carried by one straight arm at a time, while the
// legs, opened in a wide straddle, sweep circles around it. Everything is a
// function of the spin angle, so the move loops seamlessly:
//  - the whole body turns around the vertical axis (rootYaw), pivoting on the
//    planted hand (supportPinning: the hand on the floor does not slide);
//  - it rolls onto the supporting arm (rootRoll) and its hips rise and fall;
//  - the hands alternate: the free arm is raised to the side to let the legs
//    pass, comes back under the chest, then reaches down next to the other
//    hand just before that one lifts off (short double support).
class ThomasFlareAnimation : public IAnimation
{
public:
	const std::string& name() const override { return name_; }

	Pose sample(float time) const override
	{
		const float spin = time / CYCLE_SECONDS;	// in revolutions
		const float angle = 2.0f * math::PI * spin;
		const float leftSupport = std::sin(angle);	// > 0: the left hand carries the body

		Pose pose;
		pose.rootYaw = -360.0f * spin;
		pose.supportPinning = 1.0f;
		pose.rootPitch = BODY_PITCH + 6.0f * std::cos(2.0f * angle);
		pose.rootRoll = BODY_ROLL * leftSupport;
		pose.neckPitch = -35.0f;	// face toward the floor ahead
		pose.neckRoll = -0.5f * pose.rootRoll;

		pose.leftArm = arm(leftSupport);
		pose.rightArm = arm(-leftSupport);

		// The leg on the free side swings forward under the raised hand while
		// the other one stays back: the scissor makes the legs draw circles.
		const float scissor = LEG_SCISSOR * leftSupport;
		pose.leftLeg = {LEG_LIFT - scissor, STRADDLE, 0.0f, POINTED_TOES};
		pose.rightLeg = {LEG_LIFT + scissor, STRADDLE, 0.0f, POINTED_TOES};
		return pose;
	}

private:
	static constexpr float CYCLE_SECONDS = 1.5f;	// one revolution
	static constexpr float BODY_PITCH = 95.0f;		// hips slightly above the shoulders
	static constexpr float BODY_ROLL = 15.0f;
	static constexpr float STRADDLE = 72.0f;
	static constexpr float LEG_LIFT = -10.0f;
	static constexpr float LEG_SCISSOR = 20.0f;
	static constexpr float POINTED_TOES = -40.0f;

	// Arm timing. `support` goes from -1 (fully free) to 1 (fully carrying).
	static constexpr float DOUBLE_SUPPORT = 0.6f;	// how early the free hand comes back
	static constexpr float RETURN_LEAD = 0.6f;		// it is under the chest before it reaches down
	static constexpr float BLEND_SHARPNESS = 1.2f;

	static ArmPose arm(float support)
	{
		const float inward = math::smoothstep(0.5f + BLEND_SHARPNESS * (support + DOUBLE_SUPPORT + RETURN_LEAD));
		const float down = math::smoothstep(0.5f + BLEND_SHARPNESS * (support + DOUBLE_SUPPORT));
		const ArmPose raised = {20.0f, 90.0f, 20.0f, 10.0f};		// out to the side
		const ArmPose planted = {BODY_PITCH, -25.0f, 0.0f, -80.0f};	// straight, hand flat under the chest
		return {
			math::lerp(raised.shoulderFlex, planted.shoulderFlex, down),
			math::lerp(raised.shoulderAbduct, planted.shoulderAbduct, inward),
			math::lerp(raised.elbowFlex, planted.elbowFlex, down),
			math::lerp(raised.wristFlex, planted.wristFlex, down),
		};
	}

	std::string name_ = "Thomas flare";
};

} // namespace

std::unique_ptr<IAnimation> createThomasFlare()
{
	return std::make_unique<ThomasFlareAnimation>();
}

} // namespace clips
