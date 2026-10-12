#pragma once

#include "math/Mat4.hpp"

// Camera turning around a target point (mouse drag) at a given distance
// (mouse wheel). Pure math: no OpenGL, no window.
class OrbitCamera
{
public:
	OrbitCamera(const Vec3& target = {0.0f, 1.0f, 0.0f}, float distance = 4.5f);

	void orbit(float deltaYawDegrees, float deltaPitchDegrees);
	void zoom(float factor);	// > 1 moves away, < 1 moves closer

	Vec3 position() const;
	Mat4 viewMatrix() const;
	Mat4 projectionMatrix(float aspect) const;

	const Vec3& target() const { return target_; }
	float distance() const { return distance_; }
	float yaw() const { return yaw_; }
	float pitch() const { return pitch_; }

	static constexpr float MIN_PITCH = -10.0f;
	static constexpr float MAX_PITCH = 85.0f;
	static constexpr float MIN_DISTANCE = 1.0f;
	static constexpr float MAX_DISTANCE = 20.0f;
	static constexpr float FIELD_OF_VIEW = 45.0f;

private:
	Vec3 target_;
	float distance_;
	float yaw_ = 25.0f;	// degrees around the vertical axis, 0 = in front (+Z)
	float pitch_ = 12.0f;	// degrees above the horizon
};
