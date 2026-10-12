#include "skeleton/JointLimits.hpp"

#include "math/MathUtils.hpp"

float JointRange::clamp(float angle) const
{
	return math::clamp(angle, min, max);
}

bool JointRange::contains(float angle) const
{
	return angle >= min && angle <= max;
}

namespace {

ArmPose clampArm(const ArmPose& a, const ArmLimits& l)
{
	return {
		l.shoulderFlex.clamp(a.shoulderFlex),
		l.shoulderAbduct.clamp(a.shoulderAbduct),
		l.elbowFlex.clamp(a.elbowFlex),
		l.wristFlex.clamp(a.wristFlex),
	};
}

LegPose clampLeg(const LegPose& g, const LegLimits& l)
{
	return {
		l.hipFlex.clamp(g.hipFlex),
		l.hipAbduct.clamp(g.hipAbduct),
		l.kneeFlex.clamp(g.kneeFlex),
		l.ankleFlex.clamp(g.ankleFlex),
	};
}

bool armWithin(const ArmPose& a, const ArmLimits& l)
{
	return l.shoulderFlex.contains(a.shoulderFlex) && l.shoulderAbduct.contains(a.shoulderAbduct)
		&& l.elbowFlex.contains(a.elbowFlex) && l.wristFlex.contains(a.wristFlex);
}

bool legWithin(const LegPose& g, const LegLimits& l)
{
	return l.hipFlex.contains(g.hipFlex) && l.hipAbduct.contains(g.hipAbduct)
		&& l.kneeFlex.contains(g.kneeFlex) && l.ankleFlex.contains(g.ankleFlex);
}

} // namespace

Pose JointLimits::clamp(const Pose& pose) const
{
	Pose r = pose;
	r.neckPitch = neckPitch.clamp(pose.neckPitch);
	r.neckYaw   = neckYaw.clamp(pose.neckYaw);
	r.neckRoll  = neckRoll.clamp(pose.neckRoll);
	r.leftArm   = clampArm(pose.leftArm, arm);
	r.rightArm  = clampArm(pose.rightArm, arm);
	r.leftLeg   = clampLeg(pose.leftLeg, leg);
	r.rightLeg  = clampLeg(pose.rightLeg, leg);
	return r;
}

bool JointLimits::contains(const Pose& pose) const
{
	return neckPitch.contains(pose.neckPitch) && neckYaw.contains(pose.neckYaw)
		&& neckRoll.contains(pose.neckRoll)
		&& armWithin(pose.leftArm, arm) && armWithin(pose.rightArm, arm)
		&& legWithin(pose.leftLeg, leg) && legWithin(pose.rightLeg, leg);
}
