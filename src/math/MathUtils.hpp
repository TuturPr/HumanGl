#pragma once

namespace math {

constexpr float PI = 3.14159265358979323846f;

constexpr float radians(float degrees) { return degrees * PI / 180.0f; }
constexpr float degrees(float radians) { return radians * 180.0f / PI; }

constexpr float clamp(float value, float low, float high)
{
	return value < low ? low : (value > high ? high : value);
}

constexpr float lerp(float a, float b, float t) { return a + (b - a) * t; }

// Hermite smoothstep on [0, 1]: zero slope at both ends, used for soft transitions.
constexpr float smoothstep(float t)
{
	const float c = clamp(t, 0.0f, 1.0f);
	return c * c * (3.0f - 2.0f * c);
}

} // namespace math
