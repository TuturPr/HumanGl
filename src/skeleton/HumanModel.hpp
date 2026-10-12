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
	// The only place a part is drawn: one call per part, unit shape at the
	// origin of the current matrix.
	void drawPart(MatrixStack& stack, BodyPart part, IPartDrawer& drawer) const;

	void drawHead(MatrixStack& stack, const Pose& pose, IPartDrawer& drawer) const;
	void drawArm(MatrixStack& stack, Side side, const ArmPose& arm, IPartDrawer& drawer) const;
	void drawLeg(MatrixStack& stack, Side side, const LegPose& leg, IPartDrawer& drawer) const;

	// Draws a limb segment hanging from the current joint along -Y. It is
	// stretched past both joints by half its thickness so that the rounded
	// ends overlap and no gap appears when the joint bends.
	void drawSegment(MatrixStack& stack, BodyPart part, bool extendDistal, IPartDrawer& drawer) const;
	float boneLength(BodyPart part) const;	// 0 for a hidden bonus part

	BodyDimensions dimensions_;
	bool bonusPartsEnabled_ = true;
};
