#include "animation/KeyframeTrack.hpp"
#include "animation/clips/Clips.hpp"

namespace clips {
namespace {

ArmPose guard() { return {45.0f, 12.0f, 115.0f, 0.0f}; }		// fists up in front of the chest
ArmPose punch() { return {88.0f, -12.0f, 5.0f, 0.0f}; }		// punch at shoulder height, elbow not locked
ArmPose chamber() { return {-25.0f, 10.0f, 125.0f, 0.0f}; }	// fist pulled back to the ribs

LegPose horse() { return {55.0f, 32.0f, 75.0f, 22.0f}; }		// wide, low horse stance
LegPose ready() { return {12.0f, 8.0f, 14.0f, 4.0f}; }
LegPose kneeUp() { return {95.0f, 4.0f, 110.0f, 10.0f}; }		// kick chambered
LegPose kick() { return {88.0f, 4.0f, 0.0f, 20.0f}; }			// front kick, heel first

Pose fighter(ArmPose left, ArmPose right, LegPose leftLeg, LegPose rightLeg, float twist, float lean)
{
	Pose pose;
	pose.rootYaw = twist;
	pose.rootPitch = lean;
	pose.neckYaw = -twist;	// eyes stay on the opponent
	pose.leftArm = left;
	pose.rightArm = right;
	pose.leftLeg = leftLeg;
	pose.rightLeg = rightLeg;
	return pose;
}

// Kung-fu kata: horse stance, right punch, left punch, rise, right front
// kick (chamber, snap, re-chamber) and back down into the stance. The torso
// twists into each punch, the head keeps facing forward.
class KungFuAnimation : public IAnimation
{
public:
	const std::string& name() const override { return name_; }

	Pose sample(float time) const override { return kata_.sample(time / CYCLE_SECONDS); }

private:
	static constexpr float CYCLE_SECONDS = 3.6f;

	KeyframeTrack<Pose> kata_{{
		{0.00f, fighter(guard(), guard(), horse(), horse(), 0.0f, 6.0f)},
		{0.12f, fighter(chamber(), punch(), horse(), horse(), 18.0f, 6.0f)},
		{0.24f, fighter(punch(), chamber(), horse(), horse(), -18.0f, 6.0f)},
		{0.36f, fighter(guard(), guard(), horse(), horse(), 0.0f, 6.0f)},
		{0.46f, fighter(guard(), guard(), ready(), ready(), 0.0f, 2.0f)},
		{0.56f, fighter(guard(), guard(), ready(), kneeUp(), 0.0f, -4.0f)},
		{0.64f, fighter(guard(), chamber(), ready(), kick(), 8.0f, -12.0f)},
		{0.72f, fighter(guard(), guard(), ready(), kneeUp(), 0.0f, -4.0f)},
		{0.84f, fighter(guard(), guard(), ready(), ready(), 0.0f, 2.0f)},
	}};

	std::string name_ = "Kung-fu";
};

} // namespace

std::unique_ptr<IAnimation> createKungFu()
{
	return std::make_unique<KungFuAnimation>();
}

} // namespace clips
