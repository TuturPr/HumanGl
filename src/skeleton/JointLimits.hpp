#pragma once

#include "skeleton/Pose.hpp"

// Anatomical range of a joint, in degrees.
struct JointRange
{
	float min;
	float max;

	float clamp(float angle) const;
	bool contains(float angle) const;
};

struct ArmLimits
{
	JointRange shoulderFlex   {-60.0f, 180.0f};
	JointRange shoulderAbduct {-30.0f, 180.0f};
	JointRange elbowFlex      {0.0f, 150.0f};	// an elbow never bends backward
	JointRange wristFlex      {-90.0f, 80.0f};
};

struct LegLimits
{
	JointRange hipFlex   {-30.0f, 130.0f};
	JointRange hipAbduct {-30.0f, 90.0f};
	JointRange kneeFlex  {0.0f, 150.0f};	// a knee never bends forward
	JointRange ankleFlex {-50.0f, 30.0f};
};

// Keeps every pose physically possible. The root (torso) is free: the whole
// body may spin or tilt, only the joints between two parts are limited.
struct JointLimits
{
	JointRange neckPitch {-50.0f, 60.0f};
	JointRange neckYaw   {-80.0f, 80.0f};
	JointRange neckRoll  {-40.0f, 40.0f};
	ArmLimits  arm;
	LegLimits  leg;

	Pose clamp(const Pose& pose) const;
	bool contains(const Pose& pose) const;
};
