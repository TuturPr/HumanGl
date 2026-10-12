#include "skeleton/Character.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

namespace {

std::vector<Vec3> unitCubeCorners()
{
	std::vector<Vec3> corners;
	for (float x : {-0.5f, 0.5f})
		for (float y : {-0.5f, 0.5f})
			for (float z : {-0.5f, 0.5f})
				corners.push_back({x, y, z});
	return corners;
}

// Part drawer that only collects the world-space hull points of the body.
class HullCollector : public IPartDrawer
{
public:
	explicit HullCollector(const std::vector<Vec3>& hull) : hull_(hull) {}

	void drawUnitShape(BodyPart, const Mat4& model) override
	{
		for (const Vec3& point : hull_)
			points_.push_back(model.transformPoint(point));
	}

	const std::vector<Vec3>& points() const { return points_; }

private:
	const std::vector<Vec3>& hull_;
	std::vector<Vec3> points_;
};

Pose atOrigin(const Pose& pose)
{
	Pose centred = pose;
	centred.rootOffset = {};
	return centred;
}

} // namespace

Character::Character() : contactHull_(unitCubeCorners()) {}

Character::Character(std::vector<Vec3> contactHull) : contactHull_(std::move(contactHull))
{
	if (contactHull_.empty())
		throw std::invalid_argument("Character: the contact hull needs at least one point");
}

HumanModel& Character::model()
{
	return model_;
}

const HumanModel& Character::model() const
{
	return model_;
}

Character::Contact Character::contact(const Pose& pose) const
{
	MatrixStack stack;
	HullCollector collector(contactHull_);
	model_.draw(stack, atOrigin(pose), collector);

	float lowest = std::numeric_limits<float>::max();
	for (const Vec3& point : collector.points())
		lowest = std::min(lowest, point.y);

	// Points near the floor, weighted by how close they are to it: a point
	// fades in as it approaches the floor, so the support moves continuously
	// when the body rolls from one hand (or foot) to the other.
	Vec3 support;
	float totalWeight = 0.0f;
	for (const Vec3& point : collector.points())
	{
		const float weight = 1.0f - (point.y - lowest) / CONTACT_TOLERANCE;
		if (weight > 0.0f)
		{
			support += point * weight;
			totalWeight += weight;
		}
	}
	return {lowest, support / totalWeight};
}

void Character::draw(MatrixStack& stack, const Pose& pose, IPartDrawer& drawer) const
{
	const Contact touch = contact(pose);
	const float pinning = pose.supportPinning;

	stack.push();
	stack.translate({
		pose.rootOffset.x - touch.support.x * pinning,
		pose.rootOffset.y - touch.lowest,
		pose.rootOffset.z - touch.support.z * pinning,
	});
	model_.draw(stack, atOrigin(pose), drawer);
	stack.pop();
}
