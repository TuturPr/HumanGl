#include "skeleton/HumanModel.hpp"

#include <algorithm>

namespace {

struct LimbParts
{
	BodyPart upper;
	BodyPart lower;
	BodyPart extremity;
};

LimbParts armParts(Side side)
{
	if (side == Side::Left)
		return {BodyPart::UpperArmLeft, BodyPart::ForearmLeft, BodyPart::HandLeft};
	return {BodyPart::UpperArmRight, BodyPart::ForearmRight, BodyPart::HandRight};
}

LimbParts legParts(Side side)
{
	if (side == Side::Left)
		return {BodyPart::ThighLeft, BodyPart::LowerLegLeft, BodyPart::FootLeft};
	return {BodyPart::ThighRight, BodyPart::LowerLegRight, BodyPart::FootRight};
}

float thickness(const Vec3& size)
{
	return std::min(size.x, size.z);
}

} // namespace

BodyDimensions& HumanModel::dimensions()
{
	return dimensions_;
}

const BodyDimensions& HumanModel::dimensions() const
{
	return dimensions_;
}

bool HumanModel::bonusPartsEnabled() const
{
	return bonusPartsEnabled_;
}

void HumanModel::setBonusPartsEnabled(bool enabled)
{
	bonusPartsEnabled_ = enabled;
}

bool HumanModel::isVisible(BodyPart part) const
{
	return bonusPartsEnabled_ || !isBonusPart(part);
}

float HumanModel::boneLength(BodyPart part) const
{
	return isVisible(part) ? dimensions_.size(part).y : 0.0f;
}

void HumanModel::draw(MatrixStack& stack, const Pose& pose, IPartDrawer& drawer) const
{
	const Vec3& torso = dimensions_.size(BodyPart::Torso);

	stack.push();
	stack.translate(pose.rootOffset);
	stack.rotateY(pose.rootYaw);
	stack.rotateZ(-pose.rootRoll);
	stack.rotateX(pose.rootPitch);

	stack.push();
	stack.translate({0.0f, -torso.y / 2.0f, 0.0f});
	drawPart(stack, BodyPart::Torso, torso, drawer);
	stack.pop();

	drawHead(stack, pose, drawer);
	drawArm(stack, Side::Left, pose.leftArm, drawer);
	drawArm(stack, Side::Right, pose.rightArm, drawer);
	drawLeg(stack, Side::Left, pose.leftLeg, drawer);
	drawLeg(stack, Side::Right, pose.rightLeg, drawer);

	stack.pop();
}

void HumanModel::drawPart(MatrixStack& stack, BodyPart part, const Vec3& extent, IPartDrawer& drawer) const
{
	if (!isVisible(part))
		return;
	stack.push();
	stack.scale(extent);
	drawer.drawUnitShape(part, stack.top());
	stack.pop();
}

float HumanModel::jointOverlap(BodyPart part, BodyPart child) const
{
	const Vec3& childSize = dimensions_.size(child);
	const float own = thickness(dimensions_.size(part));
	if (!isVisible(child))
		return own / 2.0f;	// free end: just round it off
	return std::min({own, childSize.x, childSize.y, childSize.z}) / 2.0f;
}

void HumanModel::drawSegment(MatrixStack& stack, BodyPart part, float distalOverlap, IPartDrawer& drawer) const
{
	const Vec3& size = dimensions_.size(part);
	const float top = thickness(size) / 2.0f;
	const float bottom = -size.y - distalOverlap;

	stack.push();
	stack.translate({0.0f, (top + bottom) / 2.0f, 0.0f});
	drawPart(stack, part, {size.x, top - bottom, size.z}, drawer);
	stack.pop();
}

void HumanModel::drawHead(MatrixStack& stack, const Pose& pose, IPartDrawer& drawer) const
{
	const Vec3& head = dimensions_.size(BodyPart::Head);

	stack.push();
	stack.rotateY(pose.neckYaw);
	stack.rotateX(pose.neckPitch);
	stack.rotateZ(-pose.neckRoll);

	stack.push();
	stack.rotateZ(180.0f);	// the neck segment grows upward
	drawSegment(stack, BodyPart::Neck, jointOverlap(BodyPart::Neck, BodyPart::Head), drawer);
	stack.pop();

	stack.translate({0.0f, boneLength(BodyPart::Neck) + head.y / 2.0f, 0.0f});
	drawPart(stack, BodyPart::Head, head, drawer);
	stack.pop();
}

void HumanModel::drawArm(MatrixStack& stack, Side side, const ArmPose& arm, IPartDrawer& drawer) const
{
	const LimbParts parts = armParts(side);
	const float torsoWidth = dimensions_.size(BodyPart::Torso).x;
	const Vec3& upperArm = dimensions_.size(parts.upper);

	stack.push();
	// Shoulder joint, just outside the top corner of the torso; the rounded
	// top of the upper arm (half its thickness above the joint) is level with
	// the top of the torso.
	stack.translate({sideSign(side) * (torsoWidth + upperArm.x) / 2.0f, -thickness(upperArm) / 2.0f, 0.0f});
	// Flexion is applied after abduction in the bone's frame: the abducted arm
	// is then swung forward, so abduction keeps moving the hand sideways even
	// when the arm is raised in front (a hand placed under the chest).
	stack.rotateX(-arm.shoulderFlex);
	stack.rotateZ(sideSign(side) * arm.shoulderAbduct);
	drawSegment(stack, parts.upper, jointOverlap(parts.upper, parts.lower), drawer);

	// Elbow joint.
	stack.translate({0.0f, -boneLength(parts.upper), 0.0f});
	stack.rotateX(-arm.elbowFlex);
	drawSegment(stack, parts.lower, jointOverlap(parts.lower, parts.extremity), drawer);

	// Wrist joint.
	stack.translate({0.0f, -boneLength(parts.lower), 0.0f});
	stack.rotateX(-arm.wristFlex);
	drawSegment(stack, parts.extremity, 0.0f, drawer);
	stack.pop();
}

void HumanModel::drawLeg(MatrixStack& stack, Side side, const LegPose& leg, IPartDrawer& drawer) const
{
	const LimbParts parts = legParts(side);
	const Vec3& torso = dimensions_.size(BodyPart::Torso);
	const Vec3& foot = dimensions_.size(parts.extremity);
	const float thighWidth = dimensions_.size(parts.upper).x;

	stack.push();
	// Hip joint, under the bottom corner of the torso.
	stack.translate({sideSign(side) * (torso.x - thighWidth) / 2.0f, -torso.y, 0.0f});
	// Same order as the shoulder: the straddle opens the leg, flexion swings it.
	stack.rotateX(-leg.hipFlex);
	stack.rotateZ(sideSign(side) * leg.hipAbduct);
	drawSegment(stack, parts.upper, jointOverlap(parts.upper, parts.lower), drawer);

	// Knee joint: the shin folds backward.
	stack.translate({0.0f, -boneLength(parts.upper), 0.0f});
	stack.rotateX(leg.kneeFlex);
	drawSegment(stack, parts.lower, jointOverlap(parts.lower, parts.extremity), drawer);

	// Ankle joint: the foot points forward, the heel slightly behind the shin.
	stack.translate({0.0f, -boneLength(parts.lower), 0.0f});
	stack.rotateX(-leg.ankleFlex);
	stack.translate({0.0f, -foot.y / 2.0f, foot.z * 0.3f});
	drawPart(stack, parts.extremity, foot, drawer);
	stack.pop();
}
