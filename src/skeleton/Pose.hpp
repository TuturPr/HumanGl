#pragma once

#include "math/Vec3.hpp"

// Joint angles are in degrees. Sign conventions (character facing +Z):
//   flex   > 0 : the bone swings forward (knee: the shin folds backward)
//   abduct > 0 : the limb moves away from the body's midline
//   wrist / ankle flex > 0 : palm toward forearm / toes toward shin
struct ArmPose
{
	float shoulderFlex   = 0.0f;
	float shoulderAbduct = 0.0f;
	float elbowFlex      = 0.0f;
	float wristFlex      = 0.0f;
};

struct LegPose
{
	float hipFlex    = 0.0f;
	float hipAbduct  = 0.0f;
	float kneeFlex   = 0.0f;
	float ankleFlex  = 0.0f;
};

// Complete pose of the character. A pose is a plain vector of numbers, so
// poses can be added and scaled: every interpolation (lerp, splines,
// cross-fades) is just a weighted sum of poses.
struct Pose
{
	// Root = torso. rootOffset.y is the height of the lowest point of the body
	// above the floor (0 = touching the ground), x / z move it on the floor.
	Vec3  rootOffset;
	float rootYaw   = 0.0f;	// turn around the vertical axis
	float rootPitch = 0.0f;	// > 0 leans forward
	float rootRoll  = 0.0f;	// > 0 leans toward the left side
	// 0..1: how strongly the point touching the floor is pinned under
	// rootOffset.x / z. 1 makes the body pivot around its support (a hand
	// planted during a power move) instead of sliding it on the floor.
	float supportPinning = 0.0f;

	float neckPitch = 0.0f;	// > 0 looks down
	float neckYaw   = 0.0f;	// > 0 looks to the left
	float neckRoll  = 0.0f;	// > 0 tilts toward the left shoulder

	ArmPose leftArm;
	ArmPose rightArm;
	LegPose leftLeg;
	LegPose rightLeg;
};

ArmPose operator+(const ArmPose& a, const ArmPose& b);
ArmPose operator*(const ArmPose& a, float s);
LegPose operator+(const LegPose& a, const LegPose& b);
LegPose operator*(const LegPose& a, float s);
Pose operator+(const Pose& a, const Pose& b);
Pose operator*(const Pose& a, float s);

// Linear blend: t = 0 gives a, t = 1 gives b.
Pose lerp(const Pose& a, const Pose& b, float t);
