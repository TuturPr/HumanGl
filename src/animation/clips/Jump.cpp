#include "animation/KeyframeTrack.hpp"
#include "animation/clips/Clips.hpp"

#include "math/MathUtils.hpp"

#include <cmath>

namespace clips {
namespace {

Pose jumpPose(float lean, float shoulderFlex, float elbowFlex, LegPose legs, float neckPitch)
{
	Pose pose;
	pose.rootPitch = lean;
	pose.neckPitch = neckPitch;
	pose.leftArm = {shoulderFlex, 8.0f, elbowFlex, 10.0f};
	pose.rightArm = pose.leftArm;
	pose.leftLeg = legs;
	pose.rightLeg = legs;
	return pose;
}

// Vertical jump: crouch, push, flight, landing, recovery. The body shape is a
// keyframe track; the height comes from a real ballistic parabola so the
// flight accelerates and decelerates like a falling body.
class JumpAnimation : public IAnimation
{
public:
	const std::string& name() const override { return name_; }

	Pose sample(float time) const override
	{
		const float phase = time / CYCLE_SECONDS;
		Pose pose = body_.sample(phase);
		pose.rootOffset.y = flightHeight(phase - std::floor(phase));
		return pose;
	}

private:
	static constexpr float CYCLE_SECONDS = 1.6f;
	static constexpr float TAKE_OFF = 0.32f;
	static constexpr float LANDING = 0.70f;
	static constexpr float APEX_HEIGHT = 0.55f;

	static float flightHeight(float phase)
	{
		if (phase <= TAKE_OFF || phase >= LANDING)
			return 0.0f;
		const float u = (phase - TAKE_OFF) / (LANDING - TAKE_OFF);
		return 4.0f * APEX_HEIGHT * u * (1.0f - u);
	}

	// jumpPose(lean, shoulderFlex, elbowFlex, {hipFlex, hipAbduct, kneeFlex, ankleFlex}, neckPitch)
	KeyframeTrack<Pose> body_{{
		{0.00f, jumpPose(2.0f, 5.0f, 12.0f, {3.0f, 3.0f, 4.0f, 2.0f}, 2.0f)},		// standing
		{0.20f, jumpPose(32.0f, -45.0f, 20.0f, {75.0f, 6.0f, 105.0f, 28.0f}, -15.0f)},	// crouch, arms back
		{0.32f, jumpPose(6.0f, 150.0f, 10.0f, {-5.0f, 3.0f, 0.0f, -45.0f}, -5.0f)},	// push off, arms thrown up
		{0.51f, jumpPose(4.0f, 165.0f, 15.0f, {45.0f, 8.0f, 70.0f, -25.0f}, 5.0f)},	// apex, knees tucked
		{0.70f, jumpPose(8.0f, 70.0f, 20.0f, {20.0f, 5.0f, 20.0f, -5.0f}, 8.0f)},	// touch down
		{0.80f, jumpPose(28.0f, 35.0f, 25.0f, {65.0f, 6.0f, 90.0f, 25.0f}, -10.0f)},	// absorb the landing
	}};

	std::string name_ = "Jump";
};

} // namespace

std::unique_ptr<IAnimation> createJump()
{
	return std::make_unique<JumpAnimation>();
}

} // namespace clips
