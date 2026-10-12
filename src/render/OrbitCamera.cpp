#include "render/OrbitCamera.hpp"

#include "math/MathUtils.hpp"

#include <cmath>

OrbitCamera::OrbitCamera(const Vec3& target, float distance)
	: target_(target), distance_(math::clamp(distance, MIN_DISTANCE, MAX_DISTANCE))
{
}

void OrbitCamera::orbit(float deltaYawDegrees, float deltaPitchDegrees)
{
	yaw_ = std::remainder(yaw_ + deltaYawDegrees, 360.0f);
	pitch_ = math::clamp(pitch_ + deltaPitchDegrees, MIN_PITCH, MAX_PITCH);
}

void OrbitCamera::zoom(float factor)
{
	if (factor > 0.0f)
		distance_ = math::clamp(distance_ * factor, MIN_DISTANCE, MAX_DISTANCE);
}

Vec3 OrbitCamera::position() const
{
	const float yaw = math::radians(yaw_);
	const float pitch = math::radians(pitch_);
	const Vec3 offset = {
		std::cos(pitch) * std::sin(yaw),
		std::sin(pitch),
		std::cos(pitch) * std::cos(yaw),
	};
	return target_ + offset * distance_;
}

Mat4 OrbitCamera::viewMatrix() const
{
	return Mat4::lookAt(position(), target_, {0.0f, 1.0f, 0.0f});
}

Mat4 OrbitCamera::projectionMatrix(float aspect) const
{
	return Mat4::perspective(FIELD_OF_VIEW, aspect, 0.05f, 100.0f);
}
