#pragma once

#include "math/MatrixStack.hpp"
#include "skeleton/BodyDimensions.hpp"
#include "skeleton/IPartDrawer.hpp"
#include "skeleton/Pose.hpp"

// The hierarchical model. Torso is the root; its origin is the middle of the
// shoulder line. Every child is positioned from its parent's dimensions, and a
// part's scale is pushed/popped around its own draw call only, so it never
// leaks to the children:
//
//   Torso ─┬─ Neck ── Head
//          ├─ UpperArm L/R ── Forearm L/R ── Hand L/R
//          └─ Thigh L/R ───── LowerLeg L/R ── Foot L/R
class HumanModel
{
public:
	BodyDimensions& dimensions();
	const BodyDimensions& dimensions() const;

	bool bonusPartsEnabled() const;
	void setBonusPartsEnabled(bool enabled);
	bool isVisible(BodyPart part) const;

	void draw(MatrixStack& stack, const Pose& pose, IPartDrawer& drawer) const;

private:
	// THE drawing function: the only place where a part is drawn, called
	// exactly once per part. It draws the 1x1x1 unit shape at the origin of
	// the current matrix, scaled to `extent` inside its own push/pop so that
	// the scale never reaches the children.
	void drawPart(MatrixStack& stack, BodyPart part, const Vec3& extent, IPartDrawer& drawer) const;

	void drawHead(MatrixStack& stack, const Pose& pose, IPartDrawer& drawer) const;
	void drawArm(MatrixStack& stack, Side side, const ArmPose& arm, IPartDrawer& drawer) const;
	void drawLeg(MatrixStack& stack, Side side, const LegPose& leg, IPartDrawer& drawer) const;

	// Positions a limb segment hanging from the current joint along -Y, then
	// draws it with drawPart. It is stretched past its joints (by half its
	// thickness at the top and by `distalOverlap` at the bottom) so that the
	// rounded ends of neighbouring parts overlap and no gap appears when a
	// joint bends.
	void drawSegment(MatrixStack& stack, BodyPart part, float distalOverlap, IPartDrawer& drawer) const;
	// Overlap at the joint between `part` and `child`: half the thickness of
	// the thinner of the two, so the overlap never pokes out of the child.
	float jointOverlap(BodyPart part, BodyPart child) const;
	float boneLength(BodyPart part) const;	// 0 for a hidden bonus part

	BodyDimensions dimensions_;
	bool bonusPartsEnabled_ = true;
};
