#include "animation/KeyframeTrack.hpp"
#include "animation/clips/Clips.hpp"

namespace clips {
namespace {

// Pose builders: each key reads as a short description of the move.
ArmPose hipHand() { return {-10.0f, 40.0f, 110.0f, 20.0f}; }		// fist on the hip
ArmPose pointUp() { return {155.0f, 30.0f, 10.0f, 0.0f}; }			// finger to the sky
ArmPose pointDown() { return {35.0f, -25.0f, 12.0f, 0.0f}; }		// across the body to the hip
ArmPose rollHigh() { return {75.0f, -5.0f, 105.0f, 10.0f}; }		// forearms rolling in front
ArmPose rollLow() { return {50.0f, -5.0f, 115.0f, 10.0f}; }

LegPose straight() { return {4.0f, 4.0f, 8.0f, 2.0f}; }
LegPose bent() { return {22.0f, 2.0f, 38.0f, 16.0f}; }

Pose disco(ArmPose left, ArmPose right, LegPose leftLeg, LegPose rightLeg, float hips, float lookYaw, float lookPitch)
{
	Pose pose;
	pose.rootRoll = hips;
	pose.rootYaw = -0.6f * hips;
	pose.neckYaw = lookYaw;
	pose.neckPitch = lookPitch;
	pose.leftArm = left;
	pose.rightArm = right;
	pose.leftLeg = leftLeg;
	pose.rightLeg = rightLeg;
	return pose;
}

// Saturday-night disco on a 120 BPM beat (one cycle = 4 beats):
// beats 1-2: the famous point, right hand to the sky then down across the
// body, hips swaying; beats 3-4: forearms rolling with knee bounces.
class DiscoAnimation : public IAnimation
{
public:
	const std::string& name() const override { return name_; }

	Pose sample(float time) const override { return dance_.sample(time / CYCLE_SECONDS); }

private:
	static constexpr float CYCLE_SECONDS = 2.0f;

	KeyframeTrack<Pose> dance_{{
		{0.000f, disco(hipHand(), pointUp(), straight(), bent(), 7.0f, -25.0f, -25.0f)},
		{0.125f, disco(hipHand(), pointDown(), bent(), straight(), -7.0f, 15.0f, 15.0f)},
		{0.250f, disco(hipHand(), pointUp(), straight(), bent(), 7.0f, -25.0f, -25.0f)},
		{0.375f, disco(hipHand(), pointDown(), bent(), straight(), -7.0f, 15.0f, 15.0f)},
		{0.500f, disco(rollHigh(), rollLow(), bent(), bent(), 4.0f, 0.0f, 5.0f)},
		{0.625f, disco(rollLow(), rollHigh(), straight(), straight(), -4.0f, 0.0f, 0.0f)},
		{0.750f, disco(rollHigh(), rollLow(), bent(), bent(), 4.0f, 0.0f, 5.0f)},
		{0.875f, disco(rollLow(), rollHigh(), straight(), bent(), -4.0f, -10.0f, -5.0f)},
	}};

	std::string name_ = "Disco";
};

} // namespace

std::unique_ptr<IAnimation> createDisco()
{
	return std::make_unique<DiscoAnimation>();
}

} // namespace clips
