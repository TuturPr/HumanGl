#include "skeleton/Pose.hpp"

ArmPose operator+(const ArmPose& a, const ArmPose& b)
{
	return {
		a.shoulderFlex + b.shoulderFlex,
		a.shoulderAbduct + b.shoulderAbduct,
		a.elbowFlex + b.elbowFlex,
		a.wristFlex + b.wristFlex,
	};
}

ArmPose operator*(const ArmPose& a, float s)
{
	return {a.shoulderFlex * s, a.shoulderAbduct * s, a.elbowFlex * s, a.wristFlex * s};
}

LegPose operator+(const LegPose& a, const LegPose& b)
{
	return {
		a.hipFlex + b.hipFlex,
		a.hipAbduct + b.hipAbduct,
		a.kneeFlex + b.kneeFlex,
		a.ankleFlex + b.ankleFlex,
	};
}

LegPose operator*(const LegPose& a, float s)
{
	return {a.hipFlex * s, a.hipAbduct * s, a.kneeFlex * s, a.ankleFlex * s};
}

Pose operator+(const Pose& a, const Pose& b)
{
	Pose r;
	r.rootOffset = a.rootOffset + b.rootOffset;
	r.rootYaw    = a.rootYaw + b.rootYaw;
	r.rootPitch  = a.rootPitch + b.rootPitch;
	r.rootRoll   = a.rootRoll + b.rootRoll;
	r.neckPitch  = a.neckPitch + b.neckPitch;
	r.neckYaw    = a.neckYaw + b.neckYaw;
	r.neckRoll   = a.neckRoll + b.neckRoll;
	r.leftArm    = a.leftArm + b.leftArm;
	r.rightArm   = a.rightArm + b.rightArm;
	r.leftLeg    = a.leftLeg + b.leftLeg;
	r.rightLeg   = a.rightLeg + b.rightLeg;
	return r;
}

Pose operator*(const Pose& a, float s)
{
	Pose r;
	r.rootOffset = a.rootOffset * s;
	r.rootYaw    = a.rootYaw * s;
	r.rootPitch  = a.rootPitch * s;
	r.rootRoll   = a.rootRoll * s;
	r.neckPitch  = a.neckPitch * s;
	r.neckYaw    = a.neckYaw * s;
	r.neckRoll   = a.neckRoll * s;
	r.leftArm    = a.leftArm * s;
	r.rightArm   = a.rightArm * s;
	r.leftLeg    = a.leftLeg * s;
	r.rightLeg   = a.rightLeg * s;
	return r;
}

Pose lerp(const Pose& a, const Pose& b, float t)
{
	return a * (1.0f - t) + b * t;
}
