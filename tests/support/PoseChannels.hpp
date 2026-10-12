#pragma once

#include "skeleton/Pose.hpp"

#include <vector>

namespace testing_support {

// Every number of a pose in a fixed order, so tests can compare poses channel by channel.
inline std::vector<float> channels(const Pose& p)
{
	std::vector<float> c = {p.rootOffset.x, p.rootOffset.y, p.rootOffset.z,
							p.rootYaw, p.rootPitch, p.rootRoll,
							p.neckPitch, p.neckYaw, p.neckRoll};
	for (const ArmPose* a : {&p.leftArm, &p.rightArm})
		c.insert(c.end(), {a->shoulderFlex, a->shoulderAbduct, a->elbowFlex, a->wristFlex});
	for (const LegPose* l : {&p.leftLeg, &p.rightLeg})
		c.insert(c.end(), {l->hipFlex, l->hipAbduct, l->kneeFlex, l->ankleFlex});
	return c;
}

inline const char* channelName(std::size_t i)
{
	static const char* names[] = {
		"rootOffset.x", "rootOffset.y", "rootOffset.z", "rootYaw", "rootPitch", "rootRoll",
		"neckPitch", "neckYaw", "neckRoll",
		"leftArm.shoulderFlex", "leftArm.shoulderAbduct", "leftArm.elbowFlex", "leftArm.wristFlex",
		"rightArm.shoulderFlex", "rightArm.shoulderAbduct", "rightArm.elbowFlex", "rightArm.wristFlex",
		"leftLeg.hipFlex", "leftLeg.hipAbduct", "leftLeg.kneeFlex", "leftLeg.ankleFlex",
		"rightLeg.hipFlex", "rightLeg.hipAbduct", "rightLeg.kneeFlex", "rightLeg.ankleFlex"};
	return names[i];
}

} // namespace testing_support
