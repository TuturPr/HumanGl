#include "animation/KeyframeTrack.hpp"
#include "animation/clips/Clips.hpp"

#include "math/MathUtils.hpp"

#include <cmath>

namespace clips {
namespace {

// Walking in place. One gait cycle is described once for a single leg and a
// single arm; the other side plays the same curve half a cycle later, and each
// arm swings opposite to the leg on its side.
class WalkAnimation : public IAnimation
{
public:
	const std::string& name() const override { return name_; }

	Pose sample(float time) const override
	{
		const float phase = time / CYCLE_SECONDS;
		const float wave = std::sin(2.0f * math::PI * phase);

		Pose pose;
		pose.leftLeg  = gait_.sample(phase);
		pose.rightLeg = gait_.sample(phase + 0.5f);
		pose.leftArm  = armSwing_.sample(phase);
		pose.rightArm = armSwing_.sample(phase + 0.5f);

		pose.rootPitch = 4.0f;		// slight forward lean
		pose.rootYaw   = 4.0f * wave;	// shoulders turn with the swinging arm
		pose.rootRoll  = 2.0f * std::cos(2.0f * math::PI * phase * 2.0f);
		pose.neckYaw   = -pose.rootYaw;	// the head keeps looking ahead
		pose.neckPitch = 2.0f;
		return pose;
	}

private:
	static constexpr float CYCLE_SECONDS = 1.1f;

	// {hipFlex, hipAbduct, kneeFlex, ankleFlex}, phase 0 = heel strike.
	KeyframeTrack<LegPose> gait_{{
		{0.00f, {25.0f, 0.0f, 3.0f, 3.0f}},		// heel strike
		{0.12f, {20.0f, 0.0f, 15.0f, -3.0f}},	// loading response
		{0.30f, {6.0f, 0.0f, 5.0f, 5.0f}},		// mid stance
		{0.50f, {-12.0f, 0.0f, 8.0f, 10.0f}},	// terminal stance
		{0.62f, {-15.0f, 0.0f, 35.0f, -15.0f}},	// toe off
		{0.75f, {8.0f, 0.0f, 60.0f, 0.0f}},		// mid swing
		{0.88f, {26.0f, 0.0f, 22.0f, 5.0f}},	// terminal swing
	}};

	// {shoulderFlex, shoulderAbduct, elbowFlex, wristFlex}, phase 0 = arm back.
	KeyframeTrack<ArmPose> armSwing_{{
		{0.00f, {-18.0f, 6.0f, 15.0f, 5.0f}},
		{0.25f, {2.0f, 6.0f, 20.0f, 5.0f}},
		{0.50f, {22.0f, 6.0f, 32.0f, 5.0f}},
		{0.75f, {2.0f, 6.0f, 22.0f, 5.0f}},
	}};

	std::string name_ = "Walk";
};

} // namespace

std::unique_ptr<IAnimation> createWalk()
{
	return std::make_unique<WalkAnimation>();
}

} // namespace clips
